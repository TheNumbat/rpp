
#pragma once

namespace rpp {

namespace formatting {

template<Allocator A, Reflectable T>
u64 snprintf(String<A>& output, u64 idx, const char* fmt, const T& value) {
    char buffer[64] = {};
    std::snprintf(buffer, 64, fmt, value);
    return output.write(idx, String_View{buffer});
}

template<Reflectable T>
u64 measure(const T& value);

template<Allocator A, Reflectable T>
u64 write(String<A>& output, u64 idx, const T& value);

template<u64 N>
struct Record_Length {
    template<typename T>
    void apply(const Literal& name, const T& value) {
        length += String_View{name}.length();
        length += 3;
        length += measure(value);
        if(n + 1 < N) length += 2;
        n++;
    }
    u64 n = 0;
    u64 length = 0;
};

template<u64 N, Allocator A>
struct Record_Write {
    template<typename T>
    void apply(const Literal& name, const T& value) {
        idx = output.write(idx, String_View{name});
        idx = output.write(idx, " : "_v);
        idx = write(output, idx, value);
        if(n + 1 < N) idx = output.write(idx, ", "_v);
        n++;
    }
    u64 n = 0;
    u64 idx = 0;
    String<A>& output;
};

template<Reflectable T>
u64 measure(const T& value) {
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
#ifdef COMPILER_MSVC
        return std::snprintf(null, 0, "%lld", value);
#else
        return std::snprintf(null, 0, "%ld", value);
#endif
    } else if constexpr(R::kind == Kind::u8_) {
        return std::snprintf(null, 0, "%hhu", value);
    } else if constexpr(R::kind == Kind::u16_) {
        return std::snprintf(null, 0, "%hu", value);
    } else if constexpr(R::kind == Kind::u32_) {
        return std::snprintf(null, 0, "%u", value);
    } else if constexpr(R::kind == Kind::u64_) {
#ifdef COMPILER_MSVC
        return std::snprintf(null, 0, "%llu", value);
#else
        return std::snprintf(null, 0, "%lu", value);
#endif
    } else if constexpr(R::kind == Kind::f32_ || R::kind == Kind::f64_) {
        return std::snprintf(null, 0, "%f", value);
    } else if constexpr(R::kind == Kind::bool_) {
        return value ? 4 : 5;
    } else if constexpr(R::kind == Kind::array_) {
        u64 length = 2;
        for(u64 i = 0; i < R::length; i++) {
            length += measure(value[i]);
            if(i + 1 < R::length) length += 2;
        }
        return length;
    } else if constexpr(R::kind == Kind::pointer_) {
        if(value == null) return 6;
        return 2 + std::snprintf(null, 0, "%p", value);
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

template<>
inline u64 measure(const String_View& string) {
    return string.length();
}
template<Allocator A>
u64 measure(const String<A>& string) {
    return string.length();
}
template<Reflectable T>
u64 measure(const Ref<T>& ref) {
    if(ref) return 5 + measure(*ref);
    return 9;
}
template<Reflectable L, Reflectable R>
u64 measure(const Pair<L, R>& pair) {
    return 8 + measure(pair.first) + measure(pair.second);
}
template<Reflectable T>
u64 measure(const Storage<T>& storage) {
    return 9 + String_View{Reflect<T>::name}.length();
}
template<Reflectable T>
u64 measure(const Opt<T>& opt) {
    if(opt) return 5 + measure(*opt);
    return 9;
}
template<Reflectable T, u64 N>
u64 measure(const Array<T, N>& array) {
    u64 length = 2;
    for(u64 i = 0; i < N; i++) {
        length += measure(array[i]);
        if(i + 1 < N) length += 2;
    }
    return length;
}
template<Reflectable T, Allocator A>
u64 measure(const Vec<T, A>& vec) {
    u64 length = 5;
    for(u64 i = 0; i < vec.length(); i++) {
        length += measure(vec[i]);
        if(i + 1 < vec.length()) length += 2;
    }
    return length;
}
template<Reflectable T, Allocator A>
u64 measure(const Box<T, A>& box) {
    if(box) return 5 + measure(*box);
    return 9;
}
template<Reflectable T, Allocator A>
u64 measure(const Stack<T, A>& stack) {
    u64 n = 0;
    u64 length = 7;
    for(const T& item : stack) {
        length += measure(item);
        if(n + 1 < stack.length()) length += 2;
        n++;
    }
    return length;
}
template<Reflectable T, Allocator A>
u64 measure(const Queue<T, A>& queue) {
    u64 n = 0;
    u64 length = 7;
    for(const T& item : queue) {
        length += measure(item);
        if(n + 1 < queue.length()) length += 2;
        n++;
    }
    return length;
}
template<Reflectable T, Allocator A>
u64 measure(const Heap<T, A>& heap) {
    u64 n = 0;
    u64 length = 6;
    for(const T& item : heap) {
        length += measure(item);
        if(n + 1 < heap.length()) length += 2;
        n++;
    }
    return length;
}
template<Reflectable K, Reflectable V, Allocator A>
u64 measure(const Map<K, V, A>& map) {
    u64 n = 0;
    u64 length = 5;
    for(const Pair<K, V>& item : map) {
        length += 5;
        length += measure(item.first) + measure(item.second);
        if(n + 1 < map.length()) length += 2;
        n++;
    }
    return length;
}

template<Allocator A, Reflectable T>
u64 write(String<A>& output, u64 idx, const T& value) {
    using R = Reflect<T>;

    if constexpr(R::kind == Kind::void_) {
        return output.write(idx, "void"_v);
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
            idx = formatting::write(output, idx, value[i]);
            if(i + 1 < R::length) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, ']');
    } else if constexpr(R::kind == Kind::pointer_) {
        if(value == null) return output.write(idx, "(null)"_v);
        idx = output.write(idx, '(');
        idx += std::snprintf(reinterpret_cast<char* const>(output.data()) + idx,
                             output.length() - idx, "%p", value);
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

template<Allocator O>
u64 write(String<O>& output, u64 idx, const String_View& value) {
    return output.write(idx, value);
}
template<Allocator O, Allocator A>
u64 write(String<O>& output, u64 idx, const String<A>& value) {
    return output.write(idx, value);
}
template<Allocator O, Reflectable T>
u64 write(String<O>& output, u64 idx, const Ref<T>& ref) {
    if(!ref) return output.write(idx, "Ref{null}"_v);
    idx = output.write(idx, "Ref{"_v);
    idx = write(output, idx, *ref);
    return output.write(idx, '}');
}
template<Allocator O, Reflectable L, Reflectable R>
u64 write(String<O>& output, u64 idx, const Pair<L, R>& pair) {
    idx = output.write(idx, "Pair{"_v);
    idx = write(output, idx, pair.first);
    idx = output.write(idx, ", "_v);
    idx = write(output, idx, pair.second);
    return output.write(idx, '}');
}
template<Allocator O, Reflectable T>
u64 write(String<O>& output, u64 idx, const Storage<T>& storage) {
    idx = output.write(idx, "Storage<"_v);
    idx = output.write(idx, String_View{Reflect<T>::name});
    return output.write(idx, '>');
}
template<Allocator O, Reflectable T>
u64 write(String<O>& output, u64 idx, const Opt<T>& opt) {
    if(!opt) return output.write(idx, "Opt{None}"_v);
    idx = output.write(idx, "Opt{"_v);
    idx = write(output, idx, *opt);
    return output.write(idx, '}');
}
template<Allocator O, Reflectable T, u64 N>
u64 write(String<O>& output, u64 idx, const Array<T, N>& array) {
    idx = output.write(idx, '[');
    for(u64 i = 0; i < N; i++) {
        idx = write(output, idx, array[i]);
        if(i + 1 < N) idx = output.write(idx, ", "_v);
    }
    return output.write(idx, ']');
}
template<Allocator O, Reflectable T, Allocator A>
u64 write(String<O>& output, u64 idx, const Vec<T, A>& vec) {
    idx = output.write(idx, "Vec["_v);
    for(u64 i = 0; i < vec.length(); i++) {
        idx = write(output, idx, vec[i]);
        if(i + 1 < vec.length()) idx = output.write(idx, ", "_v);
    }
    return output.write(idx, ']');
}
template<Allocator O, Reflectable T, Allocator A>
u64 write(String<O>& output, u64 idx, const Box<T, A>& box) {
    if(!box) return output.write(idx, "Box{null}"_v);
    idx = output.write(idx, "Box{"_v);
    idx = write(output, idx, *box);
    return output.write(idx, '}');
}
template<Allocator O, Reflectable T, Allocator A>
u64 write(String<O>& output, u64 idx, const Stack<T, A>& stack) {
    idx = output.write(idx, "Stack["_v);
    u64 n = 0;
    for(const T& item : stack) {
        idx = write(output, idx, item);
        if(n + 1 < stack.length()) idx = output.write(idx, ", "_v);
        n++;
    }
    return output.write(idx, ']');
}
template<Allocator O, Reflectable T, Allocator A>
u64 write(String<O>& output, u64 idx, const Queue<T, A>& queue) {
    idx = output.write(idx, "Queue["_v);
    u64 n = 0;
    for(const T& item : queue) {
        idx = write(output, idx, item);
        if(n + 1 < queue.length()) idx = output.write(idx, ", "_v);
        n++;
    }
    return output.write(idx, ']');
}
template<Allocator O, Reflectable T, Allocator A>
u64 write(String<O>& output, u64 idx, const Heap<T, A>& heap) {
    idx = output.write(idx, "Heap["_v);
    u64 n = 0;
    for(const T& item : heap) {
        idx = write(output, idx, item);
        if(n + 1 < heap.length()) idx = output.write(idx, ", "_v);
        n++;
    }
    return output.write(idx, ']');
}
template<Allocator O, Reflectable K, Reflectable V, Allocator A>
u64 write(String<O>& output, u64 idx, const Map<K, V, A>& map) {
    idx = output.write(idx, "Map["_v);
    u64 n = 0;
    for(const Pair<K, V>& item : map) {
        idx = output.write(idx, "{"_v);
        idx = write(output, idx, item.first);
        idx = output.write(idx, " : "_v);
        idx = write(output, idx, item.second);
        idx = output.write(idx, '}');
        if(n + 1 < map.length()) idx = output.write(idx, ", "_v);
        n++;
    }
    return output.write(idx, ']');
}

template<Allocator A, typename... Ts>
    requires(Reflectable<Ts> && ...)
u64 write(const String_View& fmt, u64 fmt_idx, String<A>& output, u64 output_idx,
          const Ts&... args);

template<Allocator A>
u64 write(const String_View& fmt, u64 fmt_idx, String<A>& output, u64 output_idx) {
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
u64 write(const String_View& fmt, u64 fmt_idx, String<A>& output, u64 output_idx, const T& arg,
          const Ts&... args) {
    for(; fmt_idx < fmt.length(); fmt_idx++) {
        if(fmt[fmt_idx] == '%') {
            if(fmt_idx + 1 < fmt.length() && fmt[fmt_idx + 1] == '%') {
                fmt_idx++;
                output_idx = output.write(output_idx, '%');
                continue;
            }
            fmt_idx++;
            output_idx = write(output, output_idx, arg);
            output_idx = write(fmt, fmt_idx, output, output_idx, args...);
            return output_idx;
        }
        output_idx = output.write(output_idx, fmt[fmt_idx]);
    }
    return output_idx;
}

inline Pair<u64, u64> parse_fmt(const String_View& fmt) {
    u64 length = 0, args = 0;
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

    return Pair{length, args};
}

} // namespace formatting

template<typename... Ts>
    requires(Reflectable<Ts> && ...)
u64 format_length(const String_View& fmt, const Ts&... args) {
    auto [fmt_length, n_args] = formatting::parse_fmt(fmt);
    assert(n_args == sizeof...(args));
    return fmt_length + (formatting::measure(args) + ...);
}

template<>
inline u64 format_length(const String_View& fmt) {
    return formatting::parse_fmt(fmt).first;
}

template<Allocator A, typename... Ts>
    requires(Reflectable<Ts> && ...)
String<A> format(const String_View& fmt, const Ts&... args) {
    u64 length = format_length(fmt, args...);
    String<A> output{length};
    output.set_length(length);
    u64 idx = formatting::write(fmt, 0, output, 0, args...);
    assert(idx == length);
    return output;
}

} // namespace rpp
