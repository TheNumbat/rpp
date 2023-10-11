
#pragma once

#include "async.h"
#include "base.h"
#include "thread.h"

namespace rpp::Thread {

template<Allocator A>
struct Pool;

template<Allocator A = Alloc>
struct Schedule {

    explicit Schedule(Priority priority_, Pool<A>& pool_) : priority{priority_}, pool{pool_} {
    }
    template<typename R, typename RA>
    void await_suspend(std::coroutine_handle<Async::Promise<R, RA>> task) {
        pool.co_enqueue(priority, task);
    }
    void await_resume() {
    }
    bool await_ready() {
        return false;
    }

private:
    Priority priority = Priority::normal;
    Pool<A>& pool;
};

template<Allocator A = Alloc>
struct Schedule_Event {

    explicit Schedule_Event(Priority priority_, Async::Event event_, Pool<A>& pool_)
        : priority{priority_}, event{std::move(event_)}, pool{pool_} {
    }
    template<typename R, typename RA>
    void await_suspend(std::coroutine_handle<Async::Promise<R, RA>> task) {
        pool.enqueue_event(priority, std::move(event), task);
    }
    void await_resume() {
    }
    bool await_ready() {
        return event.try_wait();
    }

private:
    Priority priority = Priority::normal;
    Async::Event event;
    Pool<A>& pool;
};

template<Allocator A = Alloc>
struct Pool {

    explicit Pool() {
        auto n_threads = hardware_threads();
        for(u64 i = 0; i < n_threads; i++) {
            threads.push(Thread([this, i] {
                set_affinity(i);
                do_work();
            }));
        }
        pending_events.push(Async::Event{});
        event_thread = Thread([this] { do_events(); });
    }
    ~Pool() {
        {
            Lock jlock(jobs_mut);
            Lock elock(events_mut);
            shutdown = true;
            jobs_cond.broadcast();
            pending_events[0].signal();
        }
        event_thread.join();
        threads.clear();
        pending_events.clear();
    }

    template<typename F, typename... Args>
        requires Invocable<F, Args...>
    auto single(Priority p, F&& f, Args&&... args) -> Future<Invoke_Result<F, Args...>, Alloc> {
        assert(!shutdown);
        return enqueue(p, std::forward<F>(f), std::forward<Args>(args)...);
    }

    Schedule<A> suspend(Priority p = Priority::normal) {
        return Schedule<A>{p, *this};
    }
    Schedule_Event<A> event(Async::Event event, Priority p = Priority::normal) {
        return Schedule_Event<A>{p, std::move(event), *this};
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

    void push_job(Priority p, Box<Job_Base, Alloc> job) {
        Lock lock(jobs_mut);
        switch(p) {
        case Priority::critical:
        case Priority::high: {
            important_jobs.push(std::move(job));
        } break;
        case Priority::normal:
        case Priority::low: {
            normal_jobs.push(std::move(job));
        } break;
        }
        jobs_cond.signal();
    }

    template<typename F, typename... Args>
        requires Invocable<F, Args...>
    auto enqueue(Priority p, F&& f, Args&&... args) -> Future<Invoke_Result<F, Args...>, Alloc> {

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
        push_job(p, Box<Job_Base, Alloc>{std::move(job)});
        return future;
    }

    template<typename R, typename RA>
    void co_enqueue(Priority p, std::coroutine_handle<Async::Promise<R, RA>> coroutine) {
        Box<Co_Job<R, RA>, Alloc> job{coroutine};
        push_job(p, Box<Job_Base, Alloc>{std::move(job)});
    }

    template<typename R, typename RA>
    void enqueue_event(Priority p, Async::Event event,
                       std::coroutine_handle<Async::Promise<R, RA>> coroutine) {
        Box<Co_Job<R, RA>, Alloc> job{coroutine};
        {
            Lock lock(events_mut);
            events_to_enqueue.emplace(std::move(event),
                                      Pending_Event_State{p, Box<Job_Base, Alloc>{std::move(job)}});
        }
        pending_events[0].signal();
    }

    void do_work() {
        for(;;) {
            auto job = Box<Job_Base, Alloc>{};
            {
                Lock lock(jobs_mut);

                while(important_jobs.empty() && normal_jobs.empty() && !shutdown)
                    jobs_cond.wait(jobs_mut);

                if(shutdown) return;

                if(important_jobs.empty()) {
                    job = std::move(normal_jobs.front());
                    normal_jobs.pop();
                } else {
                    job = std::move(important_jobs.front());
                    important_jobs.pop();
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
                    if(shutdown) return;

                    for(auto& [event, state] : events_to_enqueue) {
                        pending_events.push(std::move(event));
                        pending_event_states.push(std::move(state));
                    }
                    events_to_enqueue.clear();

                    pending_events[0].reset();
                } else {
                    auto job = std::move(pending_event_states[idx - 1].continuation);
                    auto p = pending_event_states[idx - 1].p;

                    if(pending_events.length() > 2) {
                        swap(pending_events[idx], pending_events.back());
                        swap(pending_event_states[idx - 1], pending_event_states.back());
                    }
                    pending_events.pop();
                    pending_event_states.pop();

                    push_job(p, std::move(job));
                }
            }
        }
    }

    bool shutdown = false;
    Mutex jobs_mut;
    Cond jobs_cond;

    Queue<Box<Job_Base, A>, A> normal_jobs;
    Queue<Box<Job_Base, A>, A> important_jobs;
    Vec<Thread<A>, A> threads;

    struct Pending_Event_State {
        Priority p;
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
