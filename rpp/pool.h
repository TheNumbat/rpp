
#pragma once

#include "async.h"
#include "base.h"
#include "thread.h"

namespace rpp::Thread {

template<Allocator A>
struct Pool;

template<Allocator A = Alloc>
struct Schedule {

    explicit Schedule(Priority priority_, u64 affinity_, Pool<A>& pool_)
        : priority{priority_}, affinity{affinity_}, pool{pool_} {
    }
    template<typename R, typename RA>
    void await_suspend(std::coroutine_handle<Async::Promise<R, RA>> task) {
        pool.enqueue_coroutine(priority, affinity, task);
    }
    void await_resume() {
    }
    bool await_ready() {
        return false;
    }

private:
    Priority priority = Priority::normal;
    u64 affinity = 0;
    Pool<A>& pool;
};

template<Allocator A = Alloc>
struct Schedule_Event {

    explicit Schedule_Event(Priority priority_, u64 affinity_, Async::Event event_, Pool<A>& pool_)
        : priority{priority_}, affinity{affinity_}, event{std::move(event_)}, pool{pool_} {
    }
    template<typename R, typename RA>
    void await_suspend(std::coroutine_handle<Async::Promise<R, RA>> task) {
        pool.enqueue_event(priority, affinity, std::move(event), task);
    }
    void await_resume() {
    }
    bool await_ready() {
        return event.try_wait();
    }

private:
    Priority priority = Priority::normal;
    u64 affinity = 0;
    Async::Event event;
    Pool<A>& pool;
};

template<Allocator A = Alloc>
struct Pool {

    explicit Pool() : thread_states{Vec<Thread_State, A>::make(hardware_threads())} {

        auto n_threads = hardware_threads();
        assert(n_threads <= 64);
        affinity_mask = (1ll << n_threads) - 1;

        for(u64 i = 0; i < n_threads; i++) {
            threads.push(Thread([this, i] {
                set_affinity(i);
                do_work(i);
            }));
        }
        pending_events.push(Async::Event{});
        event_thread = Thread([this] { do_events(); });
    }
    ~Pool() {
        shutdown.exchange(true);
        for(auto& state : thread_states) {
            Lock lock(state.mut);
            state.cond.signal();
        }
        {
            Lock lock(events_mut);
            pending_events[0].signal();
        }
        event_thread.join();
        threads.clear();
        pending_events.clear();
    }

    template<typename F, typename... Args>
        requires Invocable<F, Args...>
    auto single(F&& f, Args&&... args) -> Future<Invoke_Result<F, Args...>, Alloc> {
        return enqueue(Priority::normal, 0, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template<typename F, typename... Args>
        requires Invocable<F, Args...>
    auto single(Priority p, u64 affinity, F&& f, Args&&... args)
        -> Future<Invoke_Result<F, Args...>, Alloc> {
        assert((affinity & affinity_mask) == affinity);
        return enqueue(p, affinity, std::forward<F>(f), std::forward<Args>(args)...);
    }

    Schedule<A> suspend(Priority p = Priority::normal, u64 affinity = 0) {
        assert((affinity & affinity_mask) == affinity);
        return Schedule<A>{p, affinity, *this};
    }
    Schedule_Event<A> event(Async::Event event, Priority p = Priority::normal, u64 affinity = 0) {
        assert((affinity & affinity_mask) == affinity);
        return Schedule_Event<A>{p, affinity, std::move(event), *this};
    }

private:
    struct Job_Base {
        virtual ~Job_Base() {
        }
        virtual void operator()() = 0;
    };

    template<Invocable F>
    struct Job : public Job_Base {
        Job(F&& f) : func{std::move(f)} {
        }
        void operator()() {
            func();
        }
        F func;
    };

    template<typename R, typename RA>
    struct Co_Job : public Job_Base {
        Co_Job(std::coroutine_handle<Async::Promise<R, RA>> coroutine_)
            : coroutine{std::move(coroutine_)} {
            coroutine.promise().reference();
        }
        ~Co_Job() {
            if(coroutine.promise().unreference()) {
                coroutine.destroy();
            }
        }
        void operator()() {
            coroutine.resume();
        }
        std::coroutine_handle<Async::Promise<R, RA>> coroutine;
    };

    void push_job(Priority p, u64 affinity_, Box<Job_Base, Alloc> job) {

        u64 affinity = affinity_ & affinity_mask;
        u64 affinity_bits = 0;
        u64 idx = Limits<u64>::max();

        if(affinity == 0) {
            affinity = affinity_mask;
            affinity_bits = thread_states.length();
        } else {
            affinity_bits = Math::popcount(affinity);
        }

        // We are not locking the queues: empty may not be accurate.
        if(p == Priority::critical) {
            for(u64 i = 0; i < thread_states.length(); i++) {
                if((affinity & (1ll << i)) && thread_states[i].important_jobs.empty()) {
                    idx = i;
                    break;
                }
            }
        } else {
            for(u64 i = 0; i < thread_states.length(); i++) {
                if((affinity & (1ll << i)) && thread_states[i].important_jobs.empty() &&
                   thread_states[i].normal_jobs.empty()) {
                    idx = i;
                    break;
                }
            }
        }

        if(idx == Limits<u64>::max()) {
            idx = static_cast<u64>(sequence.incr() * Math::PHI32) % affinity_bits;
            for(u64 i = 0; i < thread_states.length(); i++) {
                if((affinity & (1ll << i)) && idx-- == 0) {
                    idx = i;
                    break;
                }
            }
        }
        Thread_State& state = thread_states[idx];

        Lock lock(state.mut);
        switch(p) {
        case Priority::critical:
        case Priority::high: {
            state.important_jobs.push(std::move(job));
        } break;
        case Priority::normal:
        case Priority::low: {
            state.normal_jobs.push(std::move(job));
        } break;
        }
        state.cond.signal();
    }

    template<typename F, typename... Args>
        requires Invocable<F, Args...>
    auto enqueue(Priority p, u64 affinity, F&& f, Args&&... args)
        -> Future<Invoke_Result<F, Args...>, Alloc> {

        using Ret = Invoke_Result<F, Args...>;
        auto future = Future<Ret>::make();

        auto func = [future = future.dup(), f = std::move(f),
                     ... args = std::forward<Args>(args)]() mutable {
            if constexpr(Same<Ret, void>) {
                f(std::forward<Args>(args)...);
                future->fill();
            } else {
                future->fill(f(std::forward<Args>(args)...));
            }
        };
        Box<Job<decltype(func)>, Alloc> job{std::move(func)};
        push_job(p, affinity, Box<Job_Base, Alloc>{std::move(job)});
        return future;
    }

    template<typename R, typename RA>
    void enqueue_coroutine(Priority p, u64 affinity,
                           std::coroutine_handle<Async::Promise<R, RA>> coroutine) {
        Box<Co_Job<R, RA>, Alloc> job{coroutine};
        push_job(p, affinity, Box<Job_Base, Alloc>{std::move(job)});
    }

    template<typename R, typename RA>
    void enqueue_event(Priority p, u64 affinity, Async::Event event,
                       std::coroutine_handle<Async::Promise<R, RA>> coroutine) {
        Box<Co_Job<R, RA>, Alloc> job{coroutine};
        {
            Lock lock(events_mut);
            events_to_enqueue.emplace(
                std::move(event),
                Pending_Event_State{p, affinity, Box<Job_Base, Alloc>{std::move(job)}});
        }
        pending_events[0].signal();
    }

    void do_work(u64 thread_idx) {
        Thread_State& state = thread_states[thread_idx];
        for(;;) {
            auto job = Box<Job_Base, Alloc>{};
            {
                Lock lock(state.mut);

                while(state.important_jobs.empty() && state.normal_jobs.empty() && !shutdown.load())
                    state.cond.wait(state.mut);

                if(shutdown.load()) return;

                if(state.important_jobs.empty()) {
                    job = std::move(state.normal_jobs.front());
                    state.normal_jobs.pop();
                } else {
                    job = std::move(state.important_jobs.front());
                    state.important_jobs.pop();
                }
            }
            (*job)();
        }
    }

    void do_events() {
        for(;;) {
            u64 idx = Async::Event::wait_any(Slice<Async::Event>{pending_events});
            {
                if(idx == 0) {
                    Lock lock(events_mut);

                    if(shutdown.load()) return;

                    for(auto& [event, state] : events_to_enqueue) {
                        pending_events.push(std::move(event));
                        pending_event_states.push(std::move(state));
                    }
                    events_to_enqueue.clear();

                    pending_events[0].reset();
                } else {
                    auto job = std::move(pending_event_states[idx - 1].continuation);
                    auto p = pending_event_states[idx - 1].p;
                    auto affinity = pending_event_states[idx - 1].affinity;

                    if(pending_events.length() > 2) {
                        swap(pending_events[idx], pending_events.back());
                        swap(pending_event_states[idx - 1], pending_event_states.back());
                    }
                    pending_events.pop();
                    pending_event_states.pop();

                    push_job(p, affinity, std::move(job));
                }
            }
        }
    }

    Atomic shutdown, sequence;
    u64 affinity_mask = 0;

    struct Thread_State {
        Mutex mut;
        Cond cond;
        Queue<Box<Job_Base, A>, A> normal_jobs;
        Queue<Box<Job_Base, A>, A> important_jobs;
    };
    Vec<Thread_State, A> thread_states;
    Vec<Thread<A>, A> threads;

    struct Pending_Event_State {
        Priority p;
        u64 affinity;
        Box<Job_Base, A> continuation;
    };
    Vec<Async::Event, A> pending_events;
    Vec<Pending_Event_State, A> pending_event_states;
    Vec<Pair<Async::Event, Pending_Event_State>, A> events_to_enqueue;
    Thread<A> event_thread;
    Mutex events_mut;

    template<Allocator>
    friend struct Schedule;
    template<Allocator>
    friend struct Schedule_Event;
};

} // namespace rpp::Thread
