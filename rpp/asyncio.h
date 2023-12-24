
#pragma once

#include "async.h"
#include "files.h"
#include "pool.h"

namespace rpp::Async {

Task<void> wait(Pool<>& pool, u64 ms);

Task<Opt<Vec<u8, Files::Alloc>>> read(Pool<>& pool, String_View path);
Task<bool> write(Pool<>& pool, String_View path, Slice<u8> data);

} // namespace rpp::Async
