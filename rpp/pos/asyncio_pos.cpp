
#include "../asyncio.h"
#include "../files.h"

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <unistd.h>

// The file IO operations are not actually async.

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
    co_return Opt{rpp::move(data)};
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

[[nodiscard]] Task<void> wait(Pool<>& pool, u64 ms) noexcept {

    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if(fd == -1) {
        die("Failed to create timerfd: %", Log::sys_error());
    }

    itimerspec spec = {};
    spec.it_value.tv_sec = ms / 1000;
    spec.it_value.tv_nsec = (ms % 1000) * 1000000;

    if(timerfd_settime(fd, 0, &spec, null) == -1) {
        die("Failed to set timerfd: %", Log::sys_error());
    }

    co_await pool.event(Async::Event::of_sys(fd, EPOLLIN));
}

} // namespace rpp::Async
