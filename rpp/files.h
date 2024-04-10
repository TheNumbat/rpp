
#pragma once

#include "base.h"

namespace rpp::Files {

using Alloc = Mallocator<"File IO">;

using File_Time = u64;

[[nodiscard]] Opt<Vec<u8, Alloc>> read(String_View path) noexcept;
[[nodiscard]] bool write(String_View path, Slice<const u8> data) noexcept;

[[nodiscard]] Opt<File_Time> last_write_time(String_View path) noexcept;

[[nodiscard]] bool before(const File_Time& first, const File_Time& second) noexcept;

struct Write_Watcher {

    explicit Write_Watcher(String_View path) noexcept : path_(rpp::move(path)) {
        Opt<File_Time> time = last_write_time(path_);
        if(time.ok()) last_write_time_ = *time;
    }

    [[nodiscard]] String_View path() const noexcept {
        return path_;
    }

    [[nodiscard]] Opt<Vec<u8, Alloc>> read() const noexcept {
        return Files::read(path_);
    }

    [[nodiscard]] bool poll() noexcept {
        Opt<File_Time> time = last_write_time(path_);
        if(!time.ok()) return false;
        bool ret = before(last_write_time_, *time);
        last_write_time_ = *time;
        return ret;
    }

private:
    String_View path_;
    File_Time last_write_time_ = 0;
};

} // namespace rpp::Files
