
#pragma once

#include "async.h"
#include "files.h"
#include "pool.h"

namespace rpp::Async {

[[nodiscard]] Task<void> wait(Pool<>& pool, u64 ms) noexcept;

[[nodiscard]] Task<Opt<Vec<u8, Files::Alloc>>> read(Pool<>& pool, String_View path) noexcept;
[[nodiscard]] Task<bool> write(Pool<>& pool, String_View path, Slice<u8> data) noexcept;

} // namespace rpp::Async
