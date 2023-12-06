
#include "w32_util.h"

#include <cstdio>
#include <windows.h>

namespace rpp {

Pair<wchar_t*, int> utf8_to_ucs2(String_View utf8_) {

    Region_Scope(R);
    auto utf8 = utf8_.terminate<Mregion<R>>();

    constexpr int buffer_size = MAX_PATH;
    static thread_local wchar_t wbuffer[buffer_size];

    int written = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(utf8.data()),
                                      static_cast<u32>(utf8.length()), wbuffer, buffer_size);
    if(written == 0) {
        warn("Failed to convert utf8 to ucs2: %", Log::sys_error());
        return Pair{static_cast<wchar_t*>(null), 0};
    }
    assert(written <= buffer_size);

    return Pair{static_cast<wchar_t*>(wbuffer), written};
}

String_View ucs2_to_utf8(const wchar_t* ucs2, int ucs2_len) {

    constexpr int buffer_size = 256;
    static thread_local char buffer[buffer_size];

    int written = WideCharToMultiByte(CP_UTF8, 0, ucs2, ucs2_len, buffer, buffer_size, null, null);
    if(written == 0) {
        warn("Failed to convert ucs2 to utf8: %", Log::sys_error());
        return String_View{};
    }
    assert(written <= buffer_size);

    return String_View{reinterpret_cast<const u8*>(buffer), static_cast<u64>(written)};
}

String_View basic_win32_error(u32 err) {

    constexpr int buffer_size = 64;
    static thread_local char buffer[buffer_size];

    int written = std::snprintf(buffer, buffer_size, "Win32 Error: %u", err);
    assert(written > 0 && written + 1 <= buffer_size);

    return String_View{reinterpret_cast<const u8*>(buffer), static_cast<u64>(written)};
}

} // namespace rpp
