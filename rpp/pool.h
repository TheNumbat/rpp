
#pragma once

#include "async.h"
#include "base.h"
#include "thread.h"

namespace rpp::Thread {

// TOD(max):
//      asynchronous IO events (win32, io_uring)
//      queue per thread
//      set CPU affinity for individual jobs
//      work stealing

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
struct Pool {

    explicit Pool() {
        auto n_threads = hardware_threads();
        for(u64 i = 0; i < n_threads; i++) {
            threads.push(Thread([this, i] {
                set_affinity(i);
                do_work();
            }));
        }
    }
    ~Pool() {
        {
            Lock lock(jobs_mut);
            shutdown = true;
            jobs_cond.broadcast();
        }
        threads.clear();
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

        Lock lock(jobs_mut);
        push_job(p, Box<Job_Base, Alloc>{std::move(job)});
        jobs_cond.signal();
        return future;
    }

    template<typename R, typename RA>
    void co_enqueue(Priority p, std::coroutine_handle<Async::Promise<R, RA>> coroutine) {
        Box<Co_Job<R, RA>, Alloc> job{coroutine};
        Lock lock(jobs_mut);
        push_job(p, Box<Job_Base, Alloc>{std::move(job)});
        jobs_cond.signal();
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

    bool shutdown = false;
    Mutex jobs_mut;
    Cond jobs_cond;

    Queue<Box<Job_Base, A>, A> normal_jobs;
    Queue<Box<Job_Base, A>, A> important_jobs;
    Vec<Thread<A>, A> threads;

    template<Allocator>
    friend struct Schedule;
};

} // namespace rpp::Thread
