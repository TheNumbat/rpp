
#include "../asyncio.h"

namespace rpp::AsyncIO {

Async::Task<Opt<Vec<u8, Alloc>>> read(Thread::Pool<>& pool, String_View path) {

    co_return {};
}

Async::Task<bool> write(Thread::Pool<>& pool, String_View path, Slice<u8> data) {

    co_return false;
}

Async::Task<void> wait(Thread::Pool<>& pool, u64 ms) {

    co_return;
}

} // namespace rpp::AsyncIO
