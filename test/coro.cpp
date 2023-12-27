
#include "test.h"

#include <async.h>

i32 main() {

    Profile::start_thread();
    Profile::begin_frame();

    {
        Test test{"coro"_v};
        {
            int wtf_clang_format = 0;
            (void)wtf_clang_format;

            {
                auto co = []() -> Async::Task<void> {
                    info("Hello from coroutine 1");
                    co_return;
                };
                Async::Task<void> task = co();
                assert(task.done());
                task.block();
            }
            {
                auto co = []() -> Async::Task<void> {
                    co_await Async::Continue{};
                    info("Hello from coroutine 2");
                    co_return;
                };
                Async::Task<void> task = co();
                assert(task.done());
                task.block();
            }
            {
                auto co = []() -> Async::Task<void> {
                    co_await Async::Suspend{};
                    info("Hello from coroutine 3");
                    co_return;
                };
                Async::Task<void> task = co();
                assert(!task.done());
                task.resume();
                assert(task.done());
                task.block();
            }
            {
                auto co1 = []() -> Async::Task<i32> {
                    info("Hello from coroutine 4");
                    co_return 1;
                };
                auto co2 = [&co1]() -> Async::Task<i32> {
                    info("Hello from coroutine 5");
                    i32 i = co_await co1();
                    info("Coroutine 5 got %", i);
                    co_return i;
                };

                Async::Task<i32> task = co2();
                assert(task.done());
                assert(task.block() == 1);
            }
            {
                auto co1 = []() -> Async::Task<i32> {
                    info("Hello from coroutine 6");
                    co_await Async::Suspend{};
                    co_return 1;
                }();
                auto co2 = [&co1]() -> Async::Task<i32> {
                    info("Hello from coroutine 7");
                    i32 i = co_await co1;
                    info("Coroutine 7 got %", i);
                    co_return i;
                };

                Async::Task<i32> task = co2();
                assert(!task.done());
                co1.resume();
                assert(task.done());
                assert(task.block() == 1);
            }
            {
                auto co1 = []() -> Async::Task<i32> {
                    info("Hello from coroutine 8");
                    co_await Async::Suspend{};
                    co_await Async::Suspend{};
                    co_return 1;
                };

                auto job = co1();

                auto co2 = [&job]() -> Async::Task<i32> {
                    info("Hello from coroutine 9");
                    i32 i = co_await job;
                    info("Coroutine 9 got %", i);
                    co_return i;
                };

                Async::Task<i32> task = co2();
                assert(!task.done());
                job.resume();
                job.resume();
                assert(task.done());
                assert(task.block() == 1);
            }
        }
    }

    Profile::end_frame();
    Profile::finalize();

    return 0;
}
