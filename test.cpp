
#include "rpp/base.h"

#include "rpp/files.h"
#include "rpp/net.h"

#include "rpp/async.h"
#include "rpp/asyncio.h"
#include "rpp/pool.h"
#include "rpp/thread.h"

#include "rpp/range_allocator.h"
#include "rpp/vmath.h"

#include "rpp/function.h"
#include "rpp/heap.h"
#include "rpp/rc.h"
#include "rpp/stack.h"
#include "rpp/tuple.h"
#include "rpp/variant.h"

using namespace rpp;

auto lots_of_jobs(Thread::Pool<>& pool, u64 depth) -> Async::Task<u64> {
    if(depth == 0) {
        co_return 1;
    }
    co_await pool.suspend();
    auto job0 = lots_of_jobs(pool, depth - 1);
    auto job1 = lots_of_jobs(pool, depth - 1);
    co_return co_await job0 + co_await job1;
};

i32 main() {

    Thread::set_priority(Thread::Priority::high);
    Mregion::create();
    Profile::start_thread();
    Profile::begin_frame();

    {
        Thread::Pool pool;

        for(u64 i = 0; i < 1000; i++) {
            info("lol");
            lots_of_jobs(pool, 10).block();
        }
    }

    Profile::end_frame();
    Profile::end_thread();
    Profile::finalize();
    Mregion::destroy();

    return 0;
}
