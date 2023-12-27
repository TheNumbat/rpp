
#include "test.h"

#include <asyncio.h>
#include <pool.h>

auto lots_of_jobs(Async::Pool<>& pool, u64 depth) -> Async::Task<u64> {
    if(depth == 0) {
        co_return 1;
    }
    co_await pool.suspend();
    auto job0 = lots_of_jobs(pool, depth - 1);
    auto job1 = lots_of_jobs(pool, depth - 1);
    co_return co_await job0 + co_await job1;
};

i32 main() {

    Profile::start_thread();
    Profile::begin_frame();

    {
        Test test{"pool"_v};
        {
            Async::Pool pool;

            for(u64 i = 0; i < 10; i++) {
                assert(lots_of_jobs(pool, 8).block() == 256);
            }
        }
        {
            Async::Pool pool;

            {
                auto job = [&pool]() -> Async::Task<i32> {
                    co_await pool.suspend();
                    info("Hello from coroutine 1 on thread pool");
                    co_return 1;
                }();

                info("Job returned %", job.block());
            }
            {
                auto job = [&pool]() -> Async::Task<i32> {
                    co_await pool.suspend();
                    co_return 1;
                };
                job();
                job();
                job();
            }
        }
        {
            Async::Pool pool;
            {
                auto job = [&pool_ = pool]() -> Async::Task<i32> {
                    // pool will be gone after the first suspend, so we need to copy it
                    Async::Pool<>& pool = pool_;
                    co_await pool.suspend();
                    info("Hello from coroutine 4.1 on thread pool");
                    co_await pool.suspend();
                    info("Hello from coroutine 4.2 on thread pool");
                    co_await pool.suspend();
                    info("Hello from coroutine 4.3 on thread pool");
                    co_return 1;
                };
                job().block();
            }
            {
                auto job = [&pool_ = pool]() -> Async::Task<i32> {
                    // pool will be gone after the first suspend, so we need to copy it
                    Async::Pool<>& pool = pool_;
                    co_await pool.suspend();
                    co_await pool.suspend();
                    co_await pool.suspend();
                    co_return 1;
                };
                job();
                job();
                job();
                // These are OK to drop because the promises are refcounted and
                // no job can deadlock itself
            }
        }
        {
            Async::Pool pool;

            {
                auto job = [&pool_ = pool](i32 ms) -> Async::Task<i32> {
                    auto& pool = pool_;
                    co_await pool.suspend();
                    Thread::sleep(ms);
                    co_return 1;
                };
                auto job2 = [&pool_ = pool, &job_ = job]() -> Async::Task<i32> {
                    auto& pool = pool_;
                    auto& job = job_;
                    info("5.1 begin");
                    co_await pool.suspend();
                    info("5.1: co_await 1ms job");
                    i32 i = co_await job(1);
                    info("5.1: launch 0s job");
                    auto wait = job(0);
                    info("5.1: co_await 100ms job");
                    i32 j = co_await job(100);
                    info("5.1: co_await 0s job");
                    i32 k = co_await wait;
                    info("5.1 done: % % %", i, j, k);
                    co_return i + j + k;
                };

                assert(job2().block() == 3);
                // cannot start and drop another job2 because pending continuations
                // are leaked
            }
            {
                Function<Async::Task<void>(u64)> lots_of_jobs =
                    [&pool_ = pool, &lots_of_jobs_ = lots_of_jobs](u64 depth) -> Async::Task<void> {
                    auto& pool = pool_;
                    auto& lots_of_jobs = lots_of_jobs_;
                    if(depth == 0) {
                        co_return;
                    }
                    co_await pool.suspend();
                    auto job0 = lots_of_jobs(depth - 1);
                    auto job1 = lots_of_jobs(depth - 1);
                    co_await job0;
                    co_await job1;
                };

                lots_of_jobs(10).block();
            }
        }
        {
            Async::Pool pool;
            {
                auto job = [&pool_ = pool]() -> Async::Task<void> {
                    auto& pool = pool_;
                    info("coWaiting 100ms.");
                    co_await Async::wait(pool, 100);
                    info("coWaited 100ms.");
                    co_return;
                };

                job().block();
                info("Waited 100ms.");
            }
        }
        {
            Async::Pool pool;
            {
                Vec<u8> large(4096 + 1);
                large.resize(4096 + 1);
                Async::write(pool, "large_file"_v, Slice<u8>{large}).block();
            }
            {
                auto file = Async::read(pool, "large_file"_v).block();
                info("Read file: %", file->length());
            }
            {
                auto job = [&pool_ = pool]() -> Async::Task<Vec<u8, Files::Alloc>> {
                    auto& pool = pool_;
                    info("coReading file.");
                    auto file = co_await Async::read(pool, "large_file"_v);
                    info("coRead file: %", file->length());
                    co_return move(*file);
                };

                auto file = job().block();
                info("Read file: %", file.length());
            }
        }
    }

    Profile::end_frame();
    Profile::finalize();

    return 0;
}
