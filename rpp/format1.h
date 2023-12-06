
#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

namespace Format {

template<Enum E>
constexpr Literal enum_name(E value) {
    Literal ret{"Invalid"};
    iterate_enum<E>([&](const Literal& check, const E& check_value) {
        if(value == check_value) {
            ret = check;
        }
    });
    return ret;
}

inline Opt<Pair<i64, String_View>> parse_i64(String_View input) {
    Region_Scope(R);
    auto term = input.terminate<Mregion<R>>();
    const char* start = reinterpret_cast<const char*>(term.data());
    char* end = null;
    i64 ret = Std::strtoll(start, &end, 10);
    if(start == end) {
        return {};
    }
    String_View rest = input.sub(end - start, input.length());
    return Opt<Pair<i64, String_View>>{Pair{ret, std::move(rest)}};
}

inline Opt<Pair<f32, String_View>> parse_f32(String_View input) {
    Region_Scope(R);
    auto term = input.terminate<Mregion<R>>();
    const char* start = reinterpret_cast<const char*>(term.data());
    char* end = null;
    f32 ret = Std::strtof(start, &end);
    if(start == end) {
        return {};
    }
    String_View rest = input.sub(end - start, input.length());
    return Opt<Pair<f32, String_View>>{Pair{ret, std::move(rest)}};
}

inline Opt<Pair<String_View, String_View>> parse_string(String_View s) {
    u64 start = 0;
    while(start < s.length() && ascii::is_whitespace(s[start])) {
        start++;
    }
    for(u64 i = start; i < s.length(); i++) {
        if((i + 1 == s.length() && start < i)) {
            return Opt<Pair<String_View, String_View>>{Pair{s.sub(start, i + 1), String_View{}}};
        }
        if(ascii::is_whitespace(s[i])) {
            return Opt<Pair<String_View, String_View>>{
                Pair{s.sub(start, i), s.sub(i + 1, s.length())}};
        }
    }
    return {};
}

template<Enum E>
Opt<Pair<E, String_View>> parse_enum(String_View s) {
    Opt<Pair<E, String_View>> ret = {};
    if(auto n = parse_string(s)) {
        auto [name, rest] = std::move(*n);
        iterate_enum<E>([&](const Literal& check, const E& check_value) {
            if(name == String_View{check}) {
                ret = Opt<Pair<E, String_View>>{Pair<E, String_View>{check_value, std::move(rest)}};
            }
        });
    }
    return ret;
}

} // namespace Format
} // namespace rpp
