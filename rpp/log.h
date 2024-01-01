
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

#ifdef RPP_COMPILER_MSVC
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define Here rpp::Log::Location::make(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#define Log_Scope rpp::Log::Scope log_scope__##__COUNTER__

#define info(fmt, ...)                                                                             \
    (void)(::rpp::Log::log(::rpp::Log::Level::info, Here, fmt##_v, ##__VA_ARGS__), 0)

#define warn(fmt, ...)                                                                             \
    (void)(::rpp::Log::log(::rpp::Log::Level::warn, Here, fmt##_v, ##__VA_ARGS__), 0)

#define die(fmt, ...)                                                                              \
    (void)(::rpp::Log::log(::rpp::Log::Level::fatal, Here, fmt##_v, ##__VA_ARGS__), DEBUG_BREAK,   \
           rpp::Libc::exit(1), 0)

#undef assert
#define assert(expr)                                                                               \
    (void)((!!(expr)) || (::rpp::Log::log(::rpp::Log::Level::fatal, Here,                          \
                                          rpp::String_View{"Assert: %"}, rpp::String_View{#expr}), \
                          DEBUG_BREAK, rpp::Libc::exit(1), 0))

#define UNREACHABLE                                                                                \
    (void)(::rpp::Log::log(::rpp::Log::Level::fatal, Here, rpp::String_View{"Unreachable"}),       \
           DEBUG_BREAK, rpp::Libc::exit(1), 0)

#define DEBUG_BREAK (::rpp::Log::debug_break())

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
    static constexpr Location make(const char (&file)[N], u64 line, const char (&function)[M]) {
        return Location{String_View{function}, String_View{file}.file_suffix(),
                        static_cast<u64>(line)};
    }

    bool operator==(const Log::Location& other) const {
        return function == other.function && file == other.file && line == other.line;
    }
};

struct Scope {
    Scope();
    ~Scope();
};

using Time = u64;

Time sys_time();
String_View sys_time_string(Time time);
String_View sys_error();

void debug_break();
void output(Level level, const Location& loc, String_View msg);

template<typename... Ts>
void log(Level level, const Location& loc, String_View fmt, const Ts&... args) {
    Region(R) output(level, move(loc), format<Mregion<R>>(fmt, args...).view());
}

} // namespace Log

namespace Reflect {

template<>
struct Refl<Log::Level> {
    using T = Log::Level;
    using underlying = u8;
    static constexpr char name[] = "Level";
    static constexpr Kind kind = Kind::enum_;
    static constexpr Log::Level default_ = Log::Level::info;
    using members = List<CASE(info), CASE(warn), CASE(fatal)>;
    static_assert(Enum<T>);
};

template<>
struct Refl<Log::Location> {
    using T = Log::Location;
    static constexpr char name[] = "Location";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(function), FIELD(file), FIELD(line)>;
};

} // namespace Reflect

namespace Hash {

template<>
struct Hash<Log::Location> {
    static u64 hash(const Log::Location& l) {
        return rpp::hash(l.file, l.function, l.line);
    }
};

} // namespace Hash

} // namespace rpp
