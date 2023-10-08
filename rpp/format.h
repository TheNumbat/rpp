
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

namespace Format {

template<Reflectable T>
struct Measure;

template<Allocator A, Reflectable T>
struct Write;

template<Allocator A, Reflectable T>
u64 snprintf(String<A>& output, u64 idx, const char* fmt, const T& value) {
    Region_Scope;
    u64 val_len = Std::snprintf(null, 0, fmt, value);
    String<Mregion> buffer{val_len + 1};
    buffer.set_length(val_len + 1);
    Std::snprintf(buffer.data(), buffer.length(), fmt, value);
    return output.write(idx, buffer.sub(0, val_len));
}

template<u64 N>
struct Record_Length {
    template<Reflectable T>
    void apply(const Literal& name, const T& value) {
        length += String_View{name}.length();
        length += 3;
        length += Measure<T>::measure(value);
        if(n + 1 < N) length += 2;
        n++;
    }
    u64 n = 0;
    u64 length = 0;
};

template<u64 N, Allocator A>
struct Record_Write {
    template<Reflectable T>
    void apply(const Literal& name, const T& value) {
        idx = output.write(idx, String_View{name});
        idx = output.write(idx, " : "_v);
        idx = Write<A, T>::write(output, idx, value);
        if(n + 1 < N) idx = output.write(idx, ", "_v);
        n++;
    }
    u64 n = 0;
    u64 idx = 0;
    String<A>& output;
};

template<Reflectable T>
struct Measure {
    static u64 measure(const T& value) {
        using R = Reflect<T>;

        if constexpr(R::kind == Kind::void_) {
            return 4;
        } else if constexpr(R::kind == Kind::char_) {
            return 1;
        } else if constexpr(R::kind == Kind::i8_) {
            return Std::snprintf(null, 0, "%hhd", value);
        } else if constexpr(R::kind == Kind::i16_) {
            return Std::snprintf(null, 0, "%hd", value);
        } else if constexpr(R::kind == Kind::i32_) {
            return Std::snprintf(null, 0, "%d", value);
        } else if constexpr(R::kind == Kind::i64_) {
#ifdef COMPILER_MSVC
            return Std::snprintf(null, 0, "%lld", value);
#else
            return Std::snprintf(null, 0, "%ld", value);
#endif
        } else if constexpr(R::kind == Kind::u8_) {
            return Std::snprintf(null, 0, "%hhu", value);
        } else if constexpr(R::kind == Kind::u16_) {
            return Std::snprintf(null, 0, "%hu", value);
        } else if constexpr(R::kind == Kind::u32_) {
            return Std::snprintf(null, 0, "%u", value);
        } else if constexpr(R::kind == Kind::u64_) {
#ifdef COMPILER_MSVC
            return Std::snprintf(null, 0, "%llu", value);
#else
            return Std::snprintf(null, 0, "%lu", value);
#endif
        } else if constexpr(R::kind == Kind::f32_ || R::kind == Kind::f64_) {
            return Std::snprintf(null, 0, "%f", value);
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
            return 2 + Std::snprintf(null, 0, "%p", value);
        } else if constexpr(R::kind == Kind::record_) {
            u64 name_len = String_View{R::name}.length();
            Record_Length<List_Length<typename R::members>> iterator;
            iterate_record(iterator, value);
            return 2 + name_len + iterator.length;
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

template<Allocator A, Reflectable T>
struct Write {
    static u64 write(String<A>& output, u64 idx, const T& value) {
        using R = Reflect<T>;

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
#ifdef COMPILER_MSVC
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
#ifdef COMPILER_MSVC
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
            idx += Std::snprintf(output.data() + idx, output.length() - idx, "%p", value);
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
            iterate_enum<T>([&](const Literal& name, const T& check) {
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
u64 write(String_View fmt, u64 fmt_idx, String<A>& output, u64 output_idx, const Ts&... args);

template<Allocator A>
u64 write(String_View fmt, u64 fmt_idx, String<A>& output, u64 output_idx) {
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
u64 write(String_View fmt, u64 fmt_idx, String<A>& output, u64 output_idx, const T& arg,
          const Ts&... args) {
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

inline u64 parse_fmt(String_View fmt, u64& args) {
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
u64 format_length(String_View fmt, const Ts&... args) {
    u64 n_args = 0;
    u64 fmt_length = Format::parse_fmt(fmt, n_args);
    assert(n_args == sizeof...(args));
    return fmt_length + (Format::Measure<Ts>::measure(args) + ...);
}

template<>
inline u64 format_length(String_View fmt) {
    u64 n_args = 0;
    return Format::parse_fmt(fmt, n_args);
}

template<Allocator A, typename... Ts>
    requires(Reflectable<Ts> && ...)
String<A> format(String_View fmt, const Ts&... args) {
    u64 length = format_length(fmt, args...);
    String<A> output{length};
    output.set_length(length);
    u64 idx = Format::write(fmt, 0, output, 0, args...);
    assert(idx == length);
    return output;
}

} // namespace rpp
