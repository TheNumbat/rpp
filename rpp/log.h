
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

#define Here rpp::Log::Location::make(std::source_location::current())
#define Log_Scope rpp::Log::Scope log_scope__##__COUNTER__

#define info(fmt, ...) (void)(rpp::Log::log(rpp::Log::Level::info, Here, fmt##_v, ##__VA_ARGS__), 0)

#define warn(fmt, ...) (void)(rpp::Log::log(rpp::Log::Level::warn, Here, fmt##_v, ##__VA_ARGS__), 0)

#define die(fmt, ...)                                                                              \
    (void)(rpp::Log::log(rpp::Log::Level::fatal, Here, fmt##_v, ##__VA_ARGS__), DEBUG_BREAK,       \
           ::exit(1), 0)

#undef assert
#define assert(expr)                                                                               \
    (void)((!!(expr)) || (rpp::Log::log(rpp::Log::Level::fatal, Here,                              \
                                        rpp::String_View{"Assert: %"}, rpp::String_View{#expr}),   \
                          DEBUG_BREAK, ::exit(1), 0))

#define UNREACHABLE                                                                                \
    (void)(rpp::Log::log(rpp::Log::Level::fatal, Here, rpp::String_View{"Unreachable"}),           \
           DEBUG_BREAK, ::exit(1), 0)

#define DEBUG_BREAK (rpp::Log::debug_break())

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
    u64 column = 0;

    static Location make(std::source_location source_location) {
        return Location{String_View{source_location.function_name()},
                        String_View{source_location.file_name()}.file_suffix(),
                        static_cast<u64>(source_location.line()),
                        static_cast<u64>(source_location.column())};
    }

    bool operator==(const Log::Location& other) const {
        return function == other.function && file == other.file && line == other.line &&
               column == other.column;
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
    Region_Scope(R);
    output(level, std::move(loc), format<Mregion<R>>(fmt, args...).view());
}

} // namespace Log

template<>
struct rpp::detail::Reflect<Log::Level> {
    using T = Log::Level;
    using underlying = u8;
    static constexpr char name[] = "Level";
    static constexpr Kind kind = Kind::enum_;
    static constexpr Log::Level default_ = Log::Level::info;
    using members = List<CASE(info), CASE(warn), CASE(fatal)>;
    static_assert(Enum<T>);
};

template<>
struct rpp::detail::Reflect<Log::Location> {
    using T = Log::Location;
    static constexpr char name[] = "Location";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(function), FIELD(file), FIELD(line), FIELD(column)>;
};

template<>
struct Hasher<Log::Location> {
    u64 hash(Log::Location l) {
        return Hash::combine(Hash::combine(rpp::hash(l.file), rpp::hash(l.function)),
                             Hash::combine(rpp::hash(l.line), rpp::hash(l.column)));
    }
};

} // namespace rpp
