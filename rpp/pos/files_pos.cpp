
#include "../files.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace rpp::Files {

[[nodiscard]] Opt<Vec<u8, Alloc>> read(String_View path_) noexcept {

    int fd = -1;
    Region(R) {
        auto path = path_.terminate<Mregion<R>>();
        fd = open(reinterpret_cast<const char*>(path.data()), O_RDONLY);
    }

    if(fd == -1) {
        warn("Failed to open file %: %", path_, Log::sys_error());
        return {};
    }

    off_t full_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    assert(full_size <= RPP_UINT32_MAX);

    Vec<u8, Alloc> data(static_cast<u64>(full_size));
    data.resize(static_cast<u64>(full_size));

    if(::read(fd, data.data(), full_size) == -1) {
        warn("Failed to read file %: %", path_, Log::sys_error());
        return {};
    }

    close(fd);
    return Opt{rpp::move(data)};
}

[[nodiscard]] bool write(String_View path_, Slice<const u8> data) noexcept {

    int fd = -1;
    Region(R) {
        auto path = path_.terminate<Mregion<R>>();
        fd = open(reinterpret_cast<const char*>(path.data()), O_RDONLY);
    }

    if(fd == -1) {
        warn("Failed to create file %: %", path_, Log::sys_error());
        return false;
    }

    if(::write(fd, data.data(), data.length()) == -1) {
        warn("Failed to write file %: %", path_, Log::sys_error());
        return false;
    }

    close(fd);
    return true;
}

[[nodiscard]] Opt<File_Time> last_write_time(String_View path_) noexcept {
    Region(R) {
        auto path = path_.terminate<Mregion<R>>();

        struct stat info;
        if(stat(reinterpret_cast<const char*>(path.data()), &info)) {
            warn("Failed to stat file %: %", path_, Log::sys_error());
            return {};
        }
        return Opt{static_cast<File_Time>(info.st_mtime)};
    }
}

[[nodiscard]] bool before(const File_Time& first, const File_Time& second) noexcept {
    return first < second;
}

} // namespace rpp::Files
