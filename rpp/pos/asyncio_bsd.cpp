
#include "../asyncio.h"
#include "../files.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// These are not actually async.

namespace rpp::Async {

[[nodiscard]] Task<Opt<Vec<u8, Files::Alloc>>> read(Pool<>& pool, String_View path_) noexcept {

    int fd = -1;
    Region(R) {
        auto path = path_.terminate<Mregion<R>>();
        fd = open(reinterpret_cast<const char*>(path.data()), O_RDONLY);
    }

    if(fd == -1) {
        warn("Failed to open file %: %", path_, Log::sys_error());
        co_return {};
    }

    off_t full_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    assert(full_size <= UINT32_MAX);

    Vec<u8, Files::Alloc> data(static_cast<u64>(full_size));
    data.resize(static_cast<u64>(full_size));

    if(::read(fd, data.data(), full_size) == -1) {
        warn("Failed to read file %: %", path_, Log::sys_error());
        co_return {};
    }

    close(fd);
    co_return Opt{move(data)};
}

[[nodiscard]] Task<bool> write(Pool<>& pool, String_View path_, Slice<u8> data) noexcept {

    int fd = -1;
    Region(R) {
        auto path = path_.terminate<Mregion<R>>();
        fd = open(reinterpret_cast<const char*>(path.data()), O_WRONLY | O_CREAT | O_TRUNC);
    }

    if(fd == -1) {
        warn("Failed to create file %: %", path_, Log::sys_error());
        co_return false;
    }

    if(::write(fd, data.data(), data.length()) == -1) {
        warn("Failed to write file %: %", path_, Log::sys_error());
        co_return false;
    }

    close(fd);
    co_return true;
}

[[nodiscard]] Task<void> wait(Pool<>&, u64 ms) noexcept {
    Thread::sleep(ms);
    co_return;
}

} // namespace rpp::Async
