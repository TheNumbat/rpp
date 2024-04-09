
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

namespace Format {

using namespace Reflect;

template<Reflectable T>
struct Measure;

template<Allocator A, Reflectable T>
struct Write;

template<Allocator A, Reflectable T>
[[nodiscard]] u64 snprintf(String<A>& output, u64 idx, const char* fmt, const T& value) noexcept {
    Region(R) {
        i32 expect = Libc::snprintf(null, 0, fmt, value);
        u64 val_len = static_cast<u64>(expect);
        String<Mregion<R>> buffer{val_len + 1};
        buffer.set_length(val_len + 1);
        assert(Libc::snprintf(buffer.data(), buffer.length(), fmt, value) == expect);
        return output.write(idx, buffer.sub(0, val_len));
    }
}

template<u64 N>
struct Record_Length {
    template<Reflectable T>
    constexpr void apply(const Literal& name, const T& value) noexcept {
        length += String_View{name}.length();
        length += 3;
        length += Measure<Decay<T>>::measure(value);
        if(n + 1 < N) length += 2;
        n++;
    }
    u64 n = 0;
    u64 length = 0;
};

template<u64 N, Allocator A>
struct Record_Write {
    template<Reflectable T>
    void apply(const Literal& name, const T& value) noexcept {
        idx = output.write(idx, String_View{name});
        idx = output.write(idx, " : "_v);
        idx = Write<A, Decay<T>>::write(output, idx, value);
        if(n + 1 < N) idx = output.write(idx, ", "_v);
        n++;
    }
    u64 n = 0;
    u64 idx = 0;
    String<A>& output;
};

template<Reflectable T>
struct Measure {
    [[nodiscard]] constexpr static u64 measure(const T& value) noexcept {
        using R = Refl<T>;

        if constexpr(R::kind == Kind::void_) {
            return 4;
        } else if constexpr(R::kind == Kind::char_) {
            return 1;
        } else if constexpr(R::kind == Kind::i8_) {
            return Libc::snprintf(null, 0, "%hhd", value);
        } else if constexpr(R::kind == Kind::i16_) {
            return Libc::snprintf(null, 0, "%hd", value);
        } else if constexpr(R::kind == Kind::i32_) {
            return Libc::snprintf(null, 0, "%d", value);
        } else if constexpr(R::kind == Kind::i64_) {
#ifdef RPP_COMPILER_MSVC
            return Libc::snprintf(null, 0, "%lld", value);
#else
            return Libc::snprintf(null, 0, "%ld", value);
#endif
        } else if constexpr(R::kind == Kind::u8_) {
            return Libc::snprintf(null, 0, "%hhu", value);
        } else if constexpr(R::kind == Kind::u16_) {
            return Libc::snprintf(null, 0, "%hu", value);
        } else if constexpr(R::kind == Kind::u32_) {
            return Libc::snprintf(null, 0, "%u", value);
        } else if constexpr(R::kind == Kind::u64_) {
#ifdef RPP_COMPILER_MSVC
            return Libc::snprintf(null, 0, "%llu", value);
#else
            return Libc::snprintf(null, 0, "%lu", value);
#endif
        } else if constexpr(R::kind == Kind::f32_ || R::kind == Kind::f64_) {
            return Libc::snprintf(null, 0, "%f", value);
        } else if constexpr(R::kind == Kind::bool_) {
            return value ? 4 : 5;
        } else if constexpr(R::kind == Kind::array_) {
            u64 length = 2;
            for(u64 i = 0; i < R::length; i++) {
                length += Measure<typename R::underlying>::measure(value[i]);
                if(i + 1 < R::length) length += 2;
            }
            return length;
        } else if constexpr(R::kind == Kind::pointer_) {
            if(value == null) return 6;
            return 2 + Libc::snprintf(null, 0, "%p", value);
        } else if constexpr(R::kind == Kind::record_) {
            u64 name_len = String_View{R::name}.length();
            Record_Length<List_Length<typename R::members>> iterator;
            iterate_record(iterator, value);
            return 2 + name_len + iterator.length;
        } else if constexpr(R::kind == Kind::enum_) {
            u64 length = String_View{R::name}.length() + 2;
            iterate_enum<T>([&](const Literal& name, T check) {
                if(value == check) {
                    length += String_View{name}.length();
                }
            });
            return length;
        }
    }
};

template<Allocator A, Reflectable T>
struct Write {
    [[nodiscard]] static u64 write(String<A>& output, u64 idx, const T& value) noexcept {
        using R = Refl<T>;

        if constexpr(R::kind == Kind::void_) {
            return output.write(idx, "void"_v);
        } else if constexpr(R::kind == Kind::char_) {
            return output.write(idx, value);
        } else if constexpr(R::kind == Kind::i8_) {
            return snprintf(output, idx, "%hhd", value);
        } else if constexpr(R::kind == Kind::i16_) {
            return snprintf(output, idx, "%hd", value);
        } else if constexpr(R::kind == Kind::i32_) {
            return snprintf(output, idx, "%d", value);
        } else if constexpr(R::kind == Kind::i64_) {
#ifdef RPP_COMPILER_MSVC
            return snprintf(output, idx, "%lld", value);
#else
            return snprintf(output, idx, "%ld", value);
#endif
        } else if constexpr(R::kind == Kind::u8_) {
            return snprintf(output, idx, "%hhu", value);
        } else if constexpr(R::kind == Kind::u16_) {
            return snprintf(output, idx, "%hu", value);
        } else if constexpr(R::kind == Kind::u32_) {
            return snprintf(output, idx, "%u", value);
        } else if constexpr(R::kind == Kind::u64_) {
#ifdef RPP_COMPILER_MSVC
            return snprintf(output, idx, "%llu", value);
#else
            return snprintf(output, idx, "%lu", value);
#endif
        } else if constexpr(R::kind == Kind::f32_ || R::kind == Kind::f64_) {
            return snprintf(output, idx, "%f", value);
        } else if constexpr(R::kind == Kind::bool_) {
            return value ? output.write(idx, "true"_v) : output.write(idx, "false"_v);
        } else if constexpr(R::kind == Kind::array_) {
            idx = output.write(idx, '[');
            for(u64 i = 0; i < R::length; i++) {
                idx = Write<A, typename R::underlying>::write(output, idx, value[i]);
                if(i + 1 < R::length) idx = output.write(idx, ", "_v);
            }
            return output.write(idx, ']');
        } else if constexpr(R::kind == Kind::pointer_) {
            if(value == null) return output.write(idx, "(null)"_v);
            idx = output.write(idx, '(');
            idx += Libc::snprintf(output.data() + idx, output.length() - idx, "%p", value);
            return output.write(idx, ')');
        } else if constexpr(R::kind == Kind::record_) {
            idx = output.write(idx, String_View{R::name});
            idx = output.write(idx, '{');
            Record_Write<List_Length<typename R::members>, A> iterator{0, idx, output};
            iterate_record(iterator, value);
            return output.write(iterator.idx, '}');
        } else if constexpr(R::kind == Kind::enum_) {
            idx = output.write(idx, String_View{R::name});
            idx = output.write(idx, "::"_v);
            iterate_enum<T>([&](const Literal& name, T check) {
                if(value == check) {
                    idx = output.write(idx, String_View{name});
                }
            });
            return idx;
        }
    }
};

template<Allocator A, typename... Ts>
    requires(Reflectable<Ts> && ...)
[[nodiscard]] u64 write(String_View fmt, u64 fmt_idx, String<A>& output, u64 output_idx,
                        const Ts&... args) noexcept;

template<Allocator A>
[[nodiscard]] u64 write(String_View fmt, u64 fmt_idx, String<A>& output, u64 output_idx) noexcept {
    for(; fmt_idx < fmt.length(); fmt_idx++) {
        if(fmt[fmt_idx] == '%') {
            assert(fmt_idx + 1 < fmt.length() && fmt[fmt_idx + 1] == '%');
            fmt_idx++;
            output_idx = output.write(output_idx, '%');
            continue;
        }
        output_idx = output.write(output_idx, fmt[fmt_idx]);
    }
    return output_idx;
}

template<Allocator A, typename T, typename... Ts>
    requires(Reflectable<T> && (Reflectable<Ts> && ...))
[[nodiscard]] u64 write(String_View fmt, u64 fmt_idx, String<A>& output, u64 output_idx,
                        const T& arg, const Ts&... args) noexcept {
    for(; fmt_idx < fmt.length(); fmt_idx++) {
        if(fmt[fmt_idx] == '%') {
            if(fmt_idx + 1 < fmt.length() && fmt[fmt_idx + 1] == '%') {
                fmt_idx++;
                output_idx = output.write(output_idx, '%');
                continue;
            }
            fmt_idx++;
            output_idx = Write<A, T>::write(output, output_idx, arg);
            output_idx = write(fmt, fmt_idx, output, output_idx, args...);
            return output_idx;
        }
        output_idx = output.write(output_idx, fmt[fmt_idx]);
    }
    return output_idx;
}

[[nodiscard]] constexpr u64 parse_fmt(String_View fmt, u64& args) noexcept {
    u64 length = 0;
    args = 0;
    for(u64 i = 0; i < fmt.length(); i++) {
        if(fmt[i] == '%') {
            if(i + 1 < fmt.length() && fmt[i + 1] == '%') {
                i++;
                length++;
                continue;
            }
            args++;
        } else {
            length++;
        }
    }

    return length;
}

} // namespace Format

template<typename... Ts>
    requires(Reflectable<Ts> && ...)
[[nodiscard]] constexpr u64 format_length(String_View fmt, const Ts&... args) noexcept {
    u64 n_args = 0;
    u64 fmt_length = Format::parse_fmt(fmt, n_args);
    assert(n_args == sizeof...(args));
    return fmt_length + (Format::Measure<Ts>::measure(args) + ...);
}

template<>
[[nodiscard]] constexpr u64 format_length(String_View fmt) noexcept {
    u64 n_args = 0;
    return Format::parse_fmt(fmt, n_args);
}

template<Allocator A, typename... Ts>
    requires(Reflectable<Ts> && ...)
[[nodiscard]] String<A> format(String_View fmt, const Ts&... args) noexcept {
    u64 length = format_length(fmt, args...);
    String<A> output{length};
    output.set_length(length);
    u64 idx = Format::write(fmt, 0, output, 0, args...);
    assert(idx == length);
    return output;
}

template<typename T>
concept Writable = requires(String<> s) {
    { s.write(0, T{}) } -> Same<u64>;
};

template<Allocator A, typename... Ss>
    requires(Writable<Decay<Ss>> && ...)
[[nodiscard]] String<A> concat(String_View sep, const Ss&&... strings) noexcept {
    if constexpr(sizeof...(strings) == 0) {
        return String<A>{};
    } else {
        u64 length = (strings.length() + ...);
        length += sep.length() * (sizeof...(strings) - 1);

        String<A> output{length};
        output.set_length(length);

        u64 idx = 0;
        ((idx = output.write(idx, strings), idx = idx < length ? output.write(idx, sep) : idx),
         ...);
        return output;
    }
}

namespace Format {

template<Reflectable T>
struct Typename {
    template<Allocator A>
    [[nodiscard]] static String<A> name() noexcept {
        return String_View{Refl<T>::name}.string<A>();
    }
};

template<Reflectable T>
struct Typename<T*> {
    template<Allocator A>
    [[nodiscard]] static String<A> name() noexcept {
        return format<A>("%*"_v, Typename<T>::template name<A>());
    }
};

template<Reflectable T, u64 N>
struct Typename<T[N]> {
    template<Allocator A>
    [[nodiscard]] static String<A> name() noexcept {
        return format<A>("%[%]"_v, Typename<T>::template name<A>(), N);
    }
};

template<template<typename> typename T, Reflectable T0>
    requires Reflectable<T<T0>>
struct Typename<T<T0>> {
    template<Allocator A>
    [[nodiscard]] static String<A> name() noexcept {
        return format<A>("%<%>"_v, String_View{Refl<T<T0>>::name},
                         Typename<T0>::template name<A>());
    }
};

template<template<typename, typename> typename T, Reflectable T0, Scalar_Allocator A0>
    requires Reflectable<T<T0, A0>>
struct Typename<T<T0, A0>> {
    template<Allocator A>
    [[nodiscard]] static String<A> name() noexcept {
        return format<A>("%<%>"_v, String_View{Refl<T<T0, A0>>::name},
                         Typename<T0>::template name<A>());
    }
};

template<template<typename, typename, typename> typename T, Reflectable T0, Reflectable T1,
         Scalar_Allocator A0>
    requires Reflectable<T<T0, T1, A0>>
struct Typename<T<T0, T1, A0>> {
    template<Allocator A>
    [[nodiscard]] static String<A> name() noexcept {
        return format<A>("%<%, %>"_v, String_View{Refl<T<T0, T1, A0>>::name},
                         Typename<T0>::template name<A>(), Typename<T1>::template name<A>());
    }
};

template<template<typename...> typename T, Reflectable... Ts>
    requires Reflectable<T<Ts...>>
struct Typename<T<Ts...>> {
    template<Allocator A>
    [[nodiscard]] static String<A> name() noexcept {
        return format<A>("%<%>"_v, String_View{Refl<T<Ts...>>::name},
                         concat<A>(", "_v, Typename<Ts>::template name<A>()...));
    }
};

template<template<typename, u64> typename T, Reflectable T0, u64 N>
    requires Reflectable<T<T0, N>>
struct Typename<T<T0, N>> {
    template<Allocator A>
    [[nodiscard]] static String<A> name() noexcept {
        return format<A>("%<%, %>"_v, String_View{Refl<T<T0, N>>::name},
                         Typename<T0>::template name<A>(), N);
    }
};

template<template<typename, typename> typename T, Reflectable T0, typename T1>
    requires(Reflectable<T<T0, T1>> && !Allocator<T1>)
struct Typename<T<T0, T1>> {
    template<Allocator A>
    [[nodiscard]] static String<A> name() noexcept {
        return format<A>("%<%, %>"_v, String_View{Refl<T<T0, T1>>::name},
                         Typename<T0>::template name<A>(), Typename<T1>::template name<A>());
    }
};

} // namespace Format

template<Reflectable T, Allocator A = Mdefault>
[[nodiscard]] String<A> format_typename() noexcept {
    return Format::Typename<Decay<T>>::template name<A>();
}

} // namespace rpp
