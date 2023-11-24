
#include "../base.h"
#include "../log_callback.h"

#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>

#ifdef OS_WINDOWS
#include "../w32/w32_util.h"
#include <windows.h>
#endif

#ifdef OS_LINUX
#include <errno.h>
#endif

namespace rpp {

namespace Log {

struct Static_Data {
    Token next = 1;
    Map<Token, Function<Callback>, Mhidden> callbacks;

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

String_View sys_error() {

    constexpr int buffer_size = 256;
    static thread_local wchar_t wbuffer[buffer_size];

    DWORD err = GetLastError();
    if(err == 0) {
        return String_View{};
    }

    u32 written = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, null,
                                 err, LANG_USER_DEFAULT, wbuffer, buffer_size, null);
    assert(written + 1 <= buffer_size);

    if(written <= 1) {
        return basic_win32_error(err);
    }

    String_View utf8_msg = ucs2_to_utf8(wbuffer, written - 1);
    if(utf8_msg.empty()) {
        return basic_win32_error(err);
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
    constexpr int buffer_size = 256;
    static thread_local char buffer[buffer_size];
    return String_View{strerror_r(errno, buffer, buffer_size)};
}

void debug_break() {
    std::raise(SIGTRAP);
}

#endif

static_assert(sizeof(std::time_t) == sizeof(Time));
static_assert(alignof(std::time_t) <= alignof(Time));

Time sys_time() {
    return std::time(null);
}

String_View sys_time_string(Time timestamp_) {

    std::time_t timestamp = static_cast<std::time_t>(timestamp_);

    constexpr u64 buffer_size = 64;
    static thread_local char buffer[buffer_size];

    std::tm tm_info;
#ifdef OS_WINDOWS
    localtime_s(&tm_info, &timestamp);
#else
    localtime_r(&timestamp, &tm_info);
#endif

    size_t written = std::strftime(buffer, buffer_size, "[%H:%M:%S]", &tm_info);
    assert(written > 0 && written + 1 <= buffer_size);

    return String_View{reinterpret_cast<const u8*>(buffer), static_cast<u64>(written)};
}

void output(Level level, const Location& loc, String_View msg) {

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

    String_View time = sys_time_string(timer);

    printf(format_str, time.length(), time.data(), level_str, thread, loc.file.length(),
           loc.file.data(), loc.line, loc.column, g_log_indent * INDENT_SIZE, "", msg.length(),
           msg.data());
    fflush(stdout);

    for(auto& [_, callback] : g_log_data.callbacks) {
        callback(level, thread, timer, loc, msg);
    }

    if(g_log_data.file) {
        fprintf(g_log_data.file, format_str, time.length(), time.data(), level_str, thread,
                loc.file.length(), loc.file.data(), loc.line, loc.column,
                g_log_indent * INDENT_SIZE, "", msg.length(), msg.data());
        fflush(g_log_data.file);
    }
}

Scope::Scope() {
    g_log_indent++;
}

Scope::~Scope() {
    assert(g_log_indent > 0);
    g_log_indent--;
}

Token subscribe(Function<void(Level, Thread::Id, Time, Location, String_View)> f) {
    Thread::Lock lock(g_log_data.lock);
    Token t = g_log_data.next++;
    g_log_data.callbacks.insert(t, std::move(f));
    return t;
}

void unsubscribe(Token token) {
    Thread::Lock lock(g_log_data.lock);
    g_log_data.callbacks.erase(token);
    if(g_log_data.callbacks.empty()) {
        g_log_data.callbacks.~Map();
    }
}

} // namespace Log

} // namespace rpp