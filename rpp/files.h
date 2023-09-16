
#pragma once

#include "base.h"

namespace rpp::Files {

using Alloc = Mallocator<"File IO">;

using File_Time = u64;

Opt<Vec<u8, Alloc>> read(const String_View& path);
bool write(const String_View& path, const Slice<u8>& data);

Opt<File_Time> last_write_time(const String_View& path);

bool before(const File_Time& first, const File_Time& second);

struct Write_Watcher {

    explicit Write_Watcher(String_View path) : path_(std::move(path)) {
        Opt<File_Time> time = last_write_time(path);
        if(time) last_write_time_ = *time;
    }

    const String_View& path() const {
        return path_;
    }

    Opt<Vec<u8, Alloc>> read() const {
        return Files::read(path_);
    }

    bool poll() {
        Opt<File_Time> time = last_write_time(path_);
        if(!time) return false;
        bool ret = before(last_write_time_, *time);
        last_write_time_ = *time;
        return ret;
    }

private:
    String_View path_;
    File_Time last_write_time_ = 0;
};

} // namespace rpp::Files
