
#pragma once

#include "async.h"
#include "base.h"
#include "thread.h"

namespace rpp::Thread {

template<Allocator A>
struct Pool;

template<Allocator A = Alloc>
struct Schedule {

    explicit Schedule(Pool<A>& pool) : pool{pool} {
    }
    template<typename R, typename RA>
    void await_suspend(std::coroutine_handle<Async::Promise<R, RA>> task) {
        task.promise().continue_with(
            [&pool = this->pool](std::coroutine_handle<> handle) { pool.enqueue(handle); });
        pool.enqueue(task);
    }
    void await_resume() {
    }
    bool await_ready() {
        return false;
    }

private:
    Pool<A>& pool;
};

template<Allocator A = Alloc>
struct Schedule_Event {

    explicit Schedule_Event(Async::Event event, Pool<A>& pool)
        : event{std::move(event)}, pool{pool} {
    }
    template<typename R, typename RA>
    void await_suspend(std::coroutine_handle<Async::Promise<R, RA>> task) {
        task.promise().continue_with(
            [&pool = this->pool](std::coroutine_handle<> handle) { pool.enqueue(handle); });
        pool.enqueue_event(std::move(event), task);
    }
    void await_resume() {
    }
    bool await_ready() {
        return event.try_wait();
    }

private:
    Async::Event event;
    Pool<A>& pool;
};

template<Allocator A = Alloc>
struct Pool {

    explicit Pool() : thread_states{Vec<Thread_State, A>::make(hardware_threads())} {

        u64 n_threads = thread_states.length();
        assert(n_threads <= 64);

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

        for(auto& state : thread_states) {
            for(auto& job : state.jobs) {
                job.destroy();
            }
        }
    }

    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;

    Pool(Pool&&) = delete;
    Pool& operator=(Pool&&) = delete;

    Schedule<A> suspend() {
        return Schedule<A>{*this};
    }
    Schedule_Event<A> event(Async::Event event) {
        return Schedule_Event<A>{std::move(event), *this};
    }

private:
    using Job = std::coroutine_handle<>;

    void enqueue(Job job) {

        u64 idx = Limits<u64>::max();

        for(u64 i = 0; i < thread_states.length(); i++) {
            Thread_State& state = thread_states[i];
            Lock lock(state.mut);
            if(state.jobs.empty()) {
                state.jobs.push(std::move(job));
                state.cond.signal();
                return;
            }
        }

        // All queues more or less busy, choose next from low discrepancy sequence
        if(idx == Limits<u64>::max()) {
            idx = static_cast<u64>(sequence.incr() * Math::PHI32) % thread_states.length();
        }

        Thread_State& state = thread_states[idx];
        Lock lock(state.mut);
        state.jobs.push(std::move(job));
        state.cond.signal();
    }

    void enqueue_event(Async::Event event, Job job) {
        Lock lock(events_mut);
        events_to_enqueue.emplace(std::move(event), std::move(job));
        pending_events[0].signal();
    }

    void do_work(u64 thread_idx) {
        Thread_State& state = thread_states[thread_idx];
        for(;;) {
            Job job;
            {
                Lock lock(state.mut);

                while(state.jobs.empty() && !shutdown.load()) state.cond.wait(state.mut);

                if(shutdown.load()) return;

                if(!state.jobs.empty()) {
                    job = std::move(state.jobs.front());
                    state.jobs.pop();
                }
            }
            job.resume();
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
                        pending_event_jobs.push(std::move(state));
                    }
                    events_to_enqueue.clear();

                    pending_events[0].reset();
                } else {
                    auto job = std::move(pending_event_jobs[idx - 1]);

                    if(pending_events.length() > 2) {
                        rpp::swap(pending_events[idx], pending_events.back());
                        rpp::swap(pending_event_jobs[idx - 1], pending_event_jobs.back());
                    }
                    pending_events.pop();
                    pending_event_jobs.pop();

                    enqueue(job);
                }
            }
        }
    }

    Atomic shutdown, sequence;

    struct Thread_State {
        Mutex mut;
        Cond cond;
        Queue<Job, A> jobs;
    };
    Vec<Thread_State, A> thread_states;
    Vec<Thread<A>, A> threads;

    Vec<Async::Event, A> pending_events;
    Vec<Job, A> pending_event_jobs;
    Vec<Pair<Async::Event, Job>, A> events_to_enqueue;

    Thread<A> event_thread;
    Mutex events_mut;

    template<Allocator>
    friend struct Schedule;
    template<Allocator>
    friend struct Schedule_Event;
};

} // namespace rpp::Thread
