
#include "../base.h"
#include "../log_callback.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef RPP_OS_WINDOWS
#include "../w32/w32_util.h"
#include <windows.h>
#endif

#if defined RPP_OS_LINUX || defined RPP_OS_MACOS
#include <errno.h>
#include <signal.h>
#endif

namespace rpp {

namespace Log {

struct Static_Data {
    Token next = 1;
    Map<Token, Function<Callback>, Mhidden> callbacks;

    Thread::Mutex lock;
    FILE* file = null;

    Static_Data() noexcept {
#ifdef RPP_OS_WINDOWS
        if(fopen_s(&file, "debug.log", "w")) file = null;
#else
        file = fopen("debug.log", "w");
#endif
    }
    ~Static_Data() noexcept {
        if(file) fclose(file);
        file = null;
    }
};

alignas(Static_Data) static char g_log_data_[sizeof(Static_Data)];
static Static_Data& g_log_data = *(Static_Data*)g_log_data_;
static thread_local u64 g_log_indent = 0;

detail::StaticInitializer::StaticInitializer() noexcept {
    new(g_log_data_) Static_Data();
}
detail::StaticInitializer::~StaticInitializer() noexcept {
    g_log_data.~Static_Data();
}

#ifdef RPP_OS_WINDOWS

[[nodiscard]] String_View sys_error() noexcept {

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

void debug_break() noexcept {
    if(IsDebuggerPresent()) {
        __debugbreak();
    }
}

#else

[[nodiscard]] String_View sys_error() noexcept {
    constexpr int buffer_size = 256;
    static thread_local char buffer[buffer_size];
#ifdef RPP_OS_MACOS
    assert(strerror_r(errno, buffer, buffer_size) == 0);
    return String_View{buffer};
#else
    return String_View{strerror_r(errno, buffer, buffer_size)};
#endif
}

void debug_break() noexcept {
    ::raise(SIGTRAP);
}

#endif

static_assert(sizeof(::time_t) == sizeof(Time));
static_assert(alignof(::time_t) <= alignof(Time));

[[nodiscard]] Time sys_time() noexcept {
    return ::time(null);
}

[[nodiscard]] String_View sys_time_string(Time timestamp_) noexcept {

    ::time_t timestamp = static_cast<::time_t>(timestamp_);

    constexpr u64 buffer_size = 64;
    static thread_local char buffer[buffer_size];

    ::tm tm_info;
#ifdef RPP_OS_WINDOWS
    localtime_s(&tm_info, &timestamp);
#else
    localtime_r(&timestamp, &tm_info);
#endif

    size_t written = ::strftime(buffer, buffer_size, "[%H:%M:%S]", &tm_info);
    assert(written > 0 && written + 1 <= buffer_size);

    return String_View{reinterpret_cast<const u8*>(buffer), static_cast<u64>(written)};
}

void output(Level level, const Location& loc, String_View msg) noexcept {

    const char* level_str;
    const char* format_str;

    switch(level) {
    case Level::info: {
        level_str = "info";
        format_str = "%.*s [%s/%zu] [%.*s:%zu]: %*s%.*s\n";
    } break;
    case Level::warn: {
        level_str = "warn";
        format_str = "\033[0;31m%.*s [%s/%zu] [%.*s:%zu]: %*s%.*s\033[0m\n";
    } break;
    case Level::fatal: {
        level_str = "fatal";
        format_str = "\033[0;31m%.*s [%s/%zu] [%.*s:%zu]: %*s%.*s\033[0m\n";
    } break;
    default: RPP_UNREACHABLE;
    }

    Thread::Id thread = Thread::this_id();
    ::time_t timer = ::time(null);

    {
        Thread::Lock lock(g_log_data.lock);

        String_View time = sys_time_string(timer);

        printf(format_str, time.length(), time.data(), level_str, thread, loc.file.length(),
               loc.file.data(), loc.line, g_log_indent * INDENT_SIZE, "", msg.length(), msg.data());
        fflush(stdout);

        if(g_log_data.file) {
            fprintf(g_log_data.file, format_str, time.length(), time.data(), level_str, thread,
                    loc.file.length(), loc.file.data(), loc.line, g_log_indent * INDENT_SIZE, "",
                    msg.length(), msg.data());
            fflush(g_log_data.file);
        }
    }

    for(auto& [_, callback] : g_log_data.callbacks) {
        callback(level, thread, timer, loc, msg);
    }
}

Scope::Scope() noexcept {
    g_log_indent++;
}

Scope::~Scope() noexcept {
    assert(g_log_indent > 0);
    g_log_indent--;
}

[[nodiscard]] Token
subscribe(Function<void(Level, Thread::Id, Time, Location, String_View)> f) noexcept {
    Thread::Lock lock(g_log_data.lock);
    Token t = g_log_data.next++;
    g_log_data.callbacks.insert(t, rpp::move(f));
    return t;
}

void unsubscribe(Token token) noexcept {
    Thread::Lock lock(g_log_data.lock);
    g_log_data.callbacks.erase(token);
    if(g_log_data.callbacks.empty()) {
        g_log_data.callbacks.~Map();
    }
}

} // namespace Log

} // namespace rpp