
#pragma once

#include "base.h"

namespace rpp::Files {

using Alloc = Mallocator<"File IO">;

using File_Time = u64;

Opt<Vec<u8, Alloc>> read(String_View path);
bool write(String_View path, Slice<u8> data);

Opt<File_Time> last_write_time(String_View path);

bool before(const File_Time& first, const File_Time& second);

struct Write_Watcher {

    explicit Write_Watcher(String_View path) : path_(move(path)) {
        Opt<File_Time> time = last_write_time(path_);
        if(time) last_write_time_ = *time;
    }

    String_View path() const {
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
