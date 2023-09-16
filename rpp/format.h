
#pragma once

namespace rpp {

struct Formatter {
    struct Record {
        template<typename T>
        void apply(const Literal& name, const T& value) {
            n++;
            length += String_View{name}.length();
            length += 3;
            length += Formatter::length(value);
        }

        u64 n = 0;
        u64 length = 0;
    };

    template<Reflectable T>
    static u64 length(const T& value) {
        using R = Reflect<T>;

        if constexpr(R::kind == Kind::void_) {
            return 4;
        } else if constexpr(R::kind == Kind::i8_) {
            return std::snprintf(null, 0, "%hhd", value);
        } else if constexpr(R::kind == Kind::i16_) {
            return std::snprintf(null, 0, "%hd", value);
        } else if constexpr(R::kind == Kind::i32_) {
            return std::snprintf(null, 0, "%d", value);
        } else if constexpr(R::kind == Kind::i64_) {
            return std::snprintf(null, 0, "%lld", value);
        } else if constexpr(R::kind == Kind::u8_) {
            return std::snprintf(null, 0, "%hhu", value);
        } else if constexpr(R::kind == Kind::u16_) {
            return std::snprintf(null, 0, "%hu", value);
        } else if constexpr(R::kind == Kind::u32_) {
            return std::snprintf(null, 0, "%u", value);
        } else if constexpr(R::kind == Kind::u64_) {
            return std::snprintf(null, 0, "%llu", value);
        } else if constexpr(R::kind == Kind::f32_ || R::kind == Kind::f64_) {
            return std::snprintf(null, 0, "%f", value);
        } else if constexpr(R::kind == Kind::bool_) {
            return value ? 4 : 5;
        } else if constexpr(R::kind == Kind::array_) {
            u64 length = 2;
            for(u64 i = 0; i < R::length; i++) {
                length += Formatter::length(value[i]);
                if(i + 1 < R::length) length += 2;
            }
            return length;
        } else if constexpr(R::kind == Kind::pointer_) {
            if(!value) return 6;
            return 2 + std::snprintf(null, 0, "%p", value);
        } else if constexpr(R::kind == Kind::record_) {
            u64 name_len = String_View{R::name}.length();
            Record iterator;
            iterate_record(iterator, value);
            return 2 + name_len + iterator.length + (iterator.n > 1 ? 2 * (iterator.n - 1) : 0);
        } else if constexpr(R::kind == Kind::enum_) {
            u64 length = String_View{R::name}.length() + 2;
            iterate_enum<T>([&](const Literal& name, const T& check) {
                if(value == check) {
                    length += String_View{name}.length();
                }
            });
            return length;
        }
    }
};

inline u64 format_args(const String_View& fmt) {
    u64 args = 0;
    for(u64 i = 0; i < fmt.length(); i++) {
        if(fmt[i] == '%') {
            if(i + 1 < fmt.length() && fmt[i + 1] == '%') {
                i++;
                continue;
            }
            args++;
        }
    }
    return args;
}

template<typename... Ts>
    requires(Reflectable<Ts> && ...)
u64 format_length(const String_View& fmt, Ts&&... args) {
    u64 n_args = format_args(fmt);
    assert(n_args == sizeof...(args));
    return fmt.length() - n_args + (Formatter::length<Ts>(std::forward<Ts>(args)) + ...);
}

template<Allocator A, typename... Ts>
    requires(Reflectable<Ts> && ...)
String<A> format(const String_View& fmt, Ts&&... args) {
    assert(format_args(fmt) == sizeof...(args));
    return String<A>{};
}

} // namespace rpp
