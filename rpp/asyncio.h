
#pragma once

#include "async.h"
#include "files.h"
#include "pool.h"

namespace rpp::AsyncIO {

using Alloc = Files::Alloc;

Async::Task<void> wait(Thread::Pool<>& pool, u64 ms);

Async::Task<Opt<Vec<u8, Alloc>>> read(Thread::Pool<>& pool, String_View path);
Async::Task<bool> write(Thread::Pool<>& pool, String_View path, Slice<u8> data);

} // namespace rpp::AsyncIO
