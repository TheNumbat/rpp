
#include "base.h"

#ifdef OS_WINDOWS
#include <windows.h>
#endif

#ifdef OS_LINUX
#include <errno.h>
#endif

namespace rpp {

namespace Log {

struct Static_Data {
    Thread::Mutex lock;
    FILE* file = null;

    Static_Data() {
#ifdef OS_WINDOWS
        if(fopen_s(&file, "debug.log", "w")) file = null;
#else
        file = fopen("debug.log", "w");
#endif
    }
    ~Static_Data() {
        if(file) fclose(file);
        file = null;
    }
};

static Static_Data g_log_data;
static thread_local u64 g_log_indent = 0;

#ifdef OS_WINDOWS

static String_View ucs2_to_utf8(wchar_t* ucs2, int ucs2_len) {

    constexpr int buf_size = 256;
    static thread_local char buf[buf_size];

    int written = WideCharToMultiByte(CP_UTF8, 0, ucs2, ucs2_len, buf, buf_size, null, null);
    if(!written) {
        warn("Failed to convert ucs2 to utf8: %", sys_error());
        return String_View{};
    }
    assert(written > 0 && written + 1 <= buf_size);

    return String_View{reinterpret_cast<const u8*>(buf), static_cast<u64>(written + 1)};
}

String_View sys_error() {

    constexpr int buf_size = 64;
    static thread_local char buf[buf_size];

    constexpr int wbuf_size = 256;
    static thread_local wchar_t wbuf[buf_size];

    DWORD err = GetLastError();
    u32 size = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, null, err,
                              LANG_USER_DEFAULT, wbuf, wbuf_size, null);

    if(!size) {
        int written = std::snprintf(buf, buf_size, "Win32 Error: %d", err);
        assert(written > 0 && written + 1 <= buf_size);
        return String_View{reinterpret_cast<const u8*>(buf), static_cast<u64>(written + 1)};
    }

    wbuf[size - 2] = '\0';
    String_View utf8_msg = ucs2_to_utf8(wbuf, size - 1);

    if(utf8_msg.empty()) {
        int written = std::snprintf(buf, buf_size, "Win32 Error: %d", err);
        assert(written > 0 && written + 1 <= buf_size);
        return String_View{reinterpret_cast<const u8*>(buf), static_cast<u64>(written + 1)};
    }

    return utf8_msg;
}

void debug_break() {
    if(IsDebuggerPresent()) {
        __debugbreak();
    }
}

#else

String_View sys_error() {
    constexpr int buf_size = 256;
    static thread_local char buf[buf_size];
    return String_View{strerror_r(errno, buf, buf_size)};
}

void debug_break() {
    raise(SIGTRAP);
}

#endif

void output(Level level, Location loc, String_View msg) {

    const char* level_str;
    const char* format_str;

    switch(level) {
    case Level::info: {
        level_str = "info";
        format_str = "%.*s [%s/%zu] [%.*s:%zu:%zu]: %*s%.*s\n";
    } break;
    case Level::warn: {
        level_str = "warn";
        format_str = "\033[0;31m%.*s [%s/%zu] [%.*s:%zu:%zu]: %*s%.*s\033[0m\n";
    } break;
    case Level::fatal: {
        level_str = "fatal";
        format_str = "\033[0;31m%.*s [%s/%zu] [%.*s:%zu:%zu]: %*s%.*s\033[0m\n";
    } break;
    default: UNREACHABLE;
    }

    Thread::Id thread = Thread::this_id();
    std::time_t timer = std::time(null);

    Thread::Lock lock(g_log_data.lock);

    String_View time = time_string(timer);

    printf(format_str, time.length(), time.data(), level_str, thread, loc.file.length(),
           loc.file.data(), loc.line, loc.column, g_log_indent * INDENT_SIZE, "", msg.length(),
           msg.data());
    fflush(stdout);

    if(g_log_data.file) {
        fprintf(g_log_data.file, format_str, time.length(), time.data(), level_str, thread,
                loc.file.length(), loc.file.data(), loc.line, loc.column,
                g_log_indent * INDENT_SIZE, "", msg.length(), msg.data());
        fflush(g_log_data.file);
    }
}

String_View time_string(std::time_t timestamp) {

    constexpr u64 buf_size = 64;
    static thread_local char buf[buf_size];

    std::tm tm_info;
#ifdef OS_WINDOWS
    localtime_s(&tm_info, &timestamp);
#else
    localtime_r(&timestamp, &tm_info);
#endif

    size_t written = std::strftime(buf, buf_size, "[%H:%M:%S]", &tm_info);
    assert(written > 0 && written + 1 <= buf_size);

    return String_View{reinterpret_cast<const u8*>(buf), static_cast<u64>(written + 1)};
}

Scope::Scope() {
    g_log_indent++;
}

Scope::~Scope() {
    assert(g_log_indent > 0);
    g_log_indent--;
}

} // namespace Log

bool operator==(const Log::Location& a, const Log::Location& b) {
    return a.function == b.function && a.file == b.file && a.line == b.line && a.column == b.column;
}

} // namespace rpp