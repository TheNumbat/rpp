
#include "base.h"
#include "files.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace rpp::Files {

Opt<Vec<u8, Alloc>> read(String_View path) {

    int fd = open(reinterpret_cast<const char*>(path.data()), O_RDONLY);
    if(fd == -1) {
        warn("Failed to open file %: %", path, Log::sys_error());
        return {};
    }

    off_t full_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    assert(full_size <= UINT32_MAX);

    Vec<u8, Alloc> data(static_cast<u64>(full_size));
    data.resize(static_cast<u64>(full_size));

    if(::read(fd, data.data(), full_size) == -1) {
        warn("Failed to read file %: %", path, Log::sys_error());
        return {};
    }

    close(fd);
    return Opt{std::move(data)};
}

bool write(String_View path, Slice<u8> data) {

    int fd = open(reinterpret_cast<const char*>(path.data()), O_WRONLY | O_CREAT | O_TRUNC);
    if(fd == -1) {
        warn("Failed to create file %: %", path, Log::sys_error());
        return false;
    }

    if(::write(fd, data.data(), data.length()) == -1) {
        warn("Failed to write file %: %", path, Log::sys_error());
        return false;
    }

    close(fd);
    return true;
}

Opt<File_Time> last_write_time(String_View path) {

    struct stat info;
    if(stat(reinterpret_cast<const char*>(path.data()), &info)) {
        warn("Failed to stat file %: %", path, Log::sys_error());
        return {};
    }

    return Opt{static_cast<File_Time>(info.st_mtime)};
}

bool before(const File_Time& first, const File_Time& second) {
    return first < second;
}

} // namespace rpp::Files
