
#pragma once

#include "base.h"
#include "thread.h"

namespace rpp::Thread {

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
    auto single(F&& f, Args&&... args) -> Future<Invoke_Result<F, Args...>, Alloc> {
        assert(!shutdown);
        return enqueue(Priority::normal, std::forward<F>(f), std::forward<Args...>(args)...);
    }

private:
    struct Job_Base {
        virtual ~Job_Base() {
        }
        virtual void operator()() = 0;
    };

    template<typename F, typename... Args>
        requires Invocable<F, Args...>
    struct Job : public Job_Base {

        using Ret = Invoke_Result<F, Args...>;
        using Future = Future<Ret, Alloc>;
        using Tuple = std::tuple<Args...>;

        F func;
        Tuple args;
        Future result;

        Job(const Future& fut, F&& f, Tuple&& args)
            : result(fut), func(std::move(f)), args(std::move(args)) {
        }

        void operator()() {
            if constexpr(Same<Ret, void>) {
                std::apply(std::move(func), std::move(args));
                result->fill();
            } else {
                result->fill(std::apply(std::move(func), std::move(args)));
            }
        }
    };

    template<typename Job>
    void push_job(Priority p, Box<Job, Alloc> job) {
        switch(p) {
        case Priority::critical:
        case Priority::high: {
            important_jobs.push(Box<Job_Base, Alloc>(std::move(job)));
        } break;
        case Priority::normal:
        case Priority::low: {
            normal_jobs.push(Box<Job_Base, Alloc>(std::move(job)));
        } break;
        }
    }

    template<typename F, typename... Args>
        requires Invocable<F, Args...>
    auto enqueue(Priority p, F&& f, Args&&... args) -> Future<Invoke_Result<F, Args...>, Alloc> {

        using Job = Job<F, Args...>;
        auto fut = Job::Future::make();

        std::tuple<Args...> args_tuple(std::forward<Args>(args)...);
        Box<Job, Alloc> job(Job{fut, std::forward<F>(f), std::move(args_tuple)});

        Lock lock(jobs_mut);
        push_job(p, std::move(job));
        jobs_cond.signal();
        return fut;
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
    Queue<Box<Job_Base, Alloc>, Alloc> normal_jobs, important_jobs;
    Vec<Thread<Alloc>, Alloc> threads;
};

} // namespace rpp::Thread
