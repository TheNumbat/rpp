
#pragma once

#define Here Log::Location::make(std::source_location::current())
#define Log_Scope Log::Scope log_scope__##__COUNTER__

#define info(fmt, ...) (void)(Log::log(Log::Level::info, Here, fmt##_v, ##__VA_ARGS__), 0)

#define warn(fmt, ...) (void)(Log::log(Log::Level::warn, Here, fmt##_v, ##__VA_ARGS__), 0)

#define die(fmt, ...)                                                                              \
    (void)(Log::log(Log::Level::fatal, Here, fmt##_v, ##__VA_ARGS__), DEBUG_BREAK, ::exit(1), 0)

#undef assert
#define assert(expr)                                                                               \
    (void)((!!(expr)) || (Log::log(Log::Level::fatal, Here, "Assert: %"_v, #expr##_v),             \
                          DEBUG_BREAK, ::exit(1), 0))

#define UNREACHABLE                                                                                \
    (void)(Log::log(Log::Level::fatal, Here, "Unreachable"_v), DEBUG_BREAK, ::exit(1), 0)

#define DEBUG_BREAK (Log::debug_break())

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
};

struct Scope {
    Scope();
    ~Scope();
};

String_View sys_error();
void debug_break();
void output(Level level, const Location& loc, String_View msg);

template<typename... Ts>
void log(Level level, const Location& loc, String_View fmt, const Ts&... args) {
    Region_Scope;
    output(level, std::move(loc), format<Mregion>(fmt, args...).view());
}

} // namespace Log

bool operator==(const Log::Location& a, const Log::Location& b);

template<>
struct Reflect<Log::Level> {
    using T = Log::Level;
    using underlying = u8;
    static constexpr char name[] = "Level";
    static constexpr Kind kind = Kind::enum_;
    static constexpr Log::Level default_ = Log::Level::info;
    using members = List<CASE(info), CASE(warn), CASE(fatal)>;
    static_assert(Enum<T>);
};

template<>
struct Reflect<Log::Location> {
    using T = Log::Location;
    static constexpr char name[] = "Location";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(function), FIELD(file), FIELD(line), FIELD(column)>;
};

} // namespace rpp
