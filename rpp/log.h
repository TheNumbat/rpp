
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

#ifdef RPP_COMPILER_MSVC
#define RPP_PRETTY_FUNCTION __FUNCTION__
#elif defined RPP_COMPILER_CLANG
#define RPP_PRETTY_FUNCTION __PRETTY_FUNCTION__
#endif

#define RPP_HERE ::rpp::Log::Location::make(__FILE__, __LINE__, RPP_PRETTY_FUNCTION)

#define info(fmt, ...)                                                                             \
    (void)(::rpp::Log::log(::rpp::Log::Level::info, RPP_HERE, fmt##_v, ##__VA_ARGS__), 0)

#define warn(fmt, ...)                                                                             \
    (void)(::rpp::Log::log(::rpp::Log::Level::warn, RPP_HERE, fmt##_v, ##__VA_ARGS__), 0)

#define die(fmt, ...)                                                                              \
    (void)(::rpp::Log::log(::rpp::Log::Level::fatal, RPP_HERE, fmt##_v, ##__VA_ARGS__),            \
           RPP_DEBUG_BREAK, ::rpp::Libc::exit(1), 0)

#undef assert
#define assert(expr)                                                                               \
    (void)((!!(expr)) ||                                                                           \
           (::rpp::Log::log(::rpp::Log::Level::fatal, RPP_HERE, ::rpp::String_View{"Assert: %"},   \
                            ::rpp::String_View{#expr}),                                            \
            RPP_DEBUG_BREAK, ::rpp::Libc::exit(1), 0))

#define RPP_UNREACHABLE                                                                            \
    (void)(::rpp::Log::log(::rpp::Log::Level::fatal, RPP_HERE, ::rpp::String_View{"Unreachable"}), \
           RPP_DEBUG_BREAK, ::rpp::Libc::exit(1), 0)

#define RPP_DEBUG_BREAK (::rpp::Log::debug_break())

#define RPP_INDENT2(COUNTER) __log_scope_##COUNTER
#define RPP_INDENT1(COUNTER) if(::rpp::Log::Scope RPP_INDENT2(COUNTER){})

#define Log_Indent RPP_INDENT1(__COUNTER__)

namespace rpp {
namespace Log {

constexpr u64 INDENT_SIZE = 4;

enum class Level : u8 {
    info,
    warn,
    fatal,
};

struct Location {
    String_View function;
    String_View file;
    u64 line = 0;

    template<u64 N, u64 M>
    [[nodiscard]] constexpr static Location make(const char (&file)[N], u64 line,
                                                 const char (&function)[M]) noexcept {
        return Location{String_View{function}, String_View{file}.file_suffix(),
                        static_cast<u64>(line)};
    }

    [[nodiscard]] constexpr bool operator==(const Log::Location& other) const noexcept {
        return function == other.function && file == other.file && line == other.line;
    }
};

struct Scope {
    Scope() noexcept;
    ~Scope() noexcept;

    [[nodiscard]] consteval operator bool() noexcept {
        return true;
    }
};

using Time = u64;

[[nodiscard]] Time sys_time() noexcept;
[[nodiscard]] String_View sys_time_string(Time time) noexcept;
[[nodiscard]] String_View sys_error() noexcept;

void debug_break() noexcept;
void output(Level level, const Location& loc, String_View msg) noexcept;

template<typename... Ts>
void log(Level level, const Location& loc, String_View fmt, const Ts&... args) noexcept {
    Region(R) output(level, loc, format<Mregion<R>>(fmt, args...).view());
}

namespace detail {
struct StaticInitializer {
    StaticInitializer() noexcept;
    ~StaticInitializer() noexcept;
} inline static_initializer;
}; // namespace detail

} // namespace Log

RPP_NAMED_ENUM(Log::Level, "Level", info, RPP_CASE(info), RPP_CASE(warn), RPP_CASE(fatal));
RPP_NAMED_RECORD(Log::Location, "Location", RPP_FIELD(function), RPP_FIELD(file), RPP_FIELD(line));

namespace Hash {

template<>
struct Hash<Log::Location> {
    [[nodiscard]] constexpr static u64 hash(const Log::Location& l) noexcept {
        return rpp::hash(l.file, l.function, l.line);
    }
};

} // namespace Hash

} // namespace rpp
