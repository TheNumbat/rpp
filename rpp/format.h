
#pragma once

namespace rpp {

namespace formatting {

template<Allocator A, Reflectable T>
u64 snprintf(String<A>& output, u64 idx, const char* fmt, const T& value) {
    Region_Scope;
    u64 val_len = std::snprintf(null, 0, fmt, value);
    String<Mregion> buffer{val_len + 1};
    buffer.set_length(val_len + 1);
    std::snprintf(reinterpret_cast<char* const>(buffer.data()), buffer.length(), fmt, value);
    return output.write(idx, buffer.sub(0, val_len));
}

template<Reflectable T>
struct Measure;

template<Allocator A, Reflectable T>
struct Write;

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
                length += Measure<typename R::underlying>::measure(value[i]);
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
};

template<>
struct Measure<String_View> {
    inline static u64 measure(String_View string) {
        return string.length();
    }
};
template<Allocator A>
struct Measure<String<A>> {
    static u64 measure(const String<A>& string) {
        return string.length();
    }
};
template<Reflectable T>
struct Measure<Ref<T>> {
    static u64 measure(const Ref<T>& ref) {
        if(ref) return 5 + Measure<T>::measure(*ref);
        return 9;
    }
};
template<Reflectable L, Reflectable R>
struct Measure<Pair<L, R>> {
    static u64 measure(const Pair<L, R>& pair) {
        return 8 + Measure<L>::measure(pair.first) + Measure<R>::measure(pair.second);
    }
};
template<Reflectable T>
struct Measure<Storage<T>> {
    static u64 measure(const Storage<T>& storage) {
        return 9 + String_View{Reflect<T>::name}.length();
    }
};
template<Reflectable T>
struct Measure<Opt<T>> {
    static u64 measure(const Opt<T>& opt) {
        if(opt) return 5 + Measure<T>::measure(*opt);
        return 9;
    }
};
template<Reflectable T, u64 N>
struct Measure<Array<T, N>> {
    static u64 measure(const Array<T, N>& array) {
        u64 length = 2;
        for(u64 i = 0; i < N; i++) {
            length += Measure<T>::measure(array[i]);
            if(i + 1 < N) length += 2;
        }
        return length;
    }
};
template<Reflectable T, Allocator A>
struct Measure<Vec<T, A>> {
    static u64 measure(const Vec<T, A>& vec) {
        u64 length = 5;
        for(u64 i = 0; i < vec.length(); i++) {
            length += Measure<T>::measure(vec[i]);
            if(i + 1 < vec.length()) length += 2;
        }
        return length;
    }
};
template<Reflectable T, Allocator A>
struct Measure<Box<T, A>> {
    static u64 measure(const Box<T, A>& box) {
        if(box) return 5 + Measure<T>::measure(*box);
        return 9;
    }
};
template<Reflectable T, Allocator A>
struct Measure<Rc<T, A>> {
    static u64 measure(const Rc<T, A>& rc) {
        if(rc)
            return 6 + Measure<T>::measure(*rc) +
                   Measure<decltype(rc.references())>::measure(rc.references());
        return 8;
    }
};
template<Reflectable T, Allocator A>
struct Measure<Arc<T, A>> {
    static u64 measure(const Arc<T, A>& arc) {
        if(arc)
            return 7 + Measure<T>::measure(*arc) +
                   Measure<decltype(arc.references())>::measure(arc.references());
        return 9;
    }
};
template<Reflectable T, Allocator A>
struct Measure<Stack<T, A>> {
    static u64 measure(const Stack<T, A>& stack) {
        u64 n = 0;
        u64 length = 7;
        for(const T& item : stack) {
            length += Measure<T>::measure(item);
            if(n + 1 < stack.length()) length += 2;
            n++;
        }
        return length;
    }
};
template<Reflectable T, Allocator A>
struct Measure<Queue<T, A>> {
    static u64 measure(const Queue<T, A>& queue) {
        u64 n = 0;
        u64 length = 7;
        for(const T& item : queue) {
            length += Measure<T>::measure(item);
            if(n + 1 < queue.length()) length += 2;
            n++;
        }
        return length;
    }
};
template<Reflectable T, Allocator A>
struct Measure<Heap<T, A>> {
    static u64 measure(const Heap<T, A>& heap) {
        u64 n = 0;
        u64 length = 6;
        for(const T& item : heap) {
            length += Measure<T>::measure(item);
            if(n + 1 < heap.length()) length += 2;
            n++;
        }
        return length;
    }
};
template<Reflectable K, Reflectable V, Allocator A>
struct Measure<Map<K, V, A>> {
    static u64 measure(const Map<K, V, A>& map) {
        u64 n = 0;
        u64 length = 5;
        for(const Pair<K, V>& item : map) {
            length += 5;
            length += Measure<K>::measure(item.first) + Measure<V>::measure(item.second);
            if(n + 1 < map.length()) length += 2;
            n++;
        }
        return length;
    }
};
template<Float F, u64 N>
struct Measure<Math::Vect<F, N>> {
    static u64 measure(const Math::Vect<F, N>& vect) {
        u64 length = 5;
        length += Measure<u64>::measure(N);
        for(u64 i = 0; i < N; i++) {
            length += Measure<F>::measure(vect[i]);
            if(i + 1 < N) length += 2;
        }
        return length;
    }
};
template<Int I, u64 N>
struct Measure<Math::Vect<I, N>> {
    static u64 measure(const Math::Vect<I, N>& vect) {
        u64 length = 6;
        length += Measure<u64>::measure(N);
        for(u64 i = 0; i < N; i++) {
            length += Measure<I>::measure(vect[i]);
            if(i + 1 < N) length += 2;
        }
        return length;
    }
};
template<>
struct Measure<Math::Quat> {
    static u64 measure(const Math::Quat& quat) {
        u64 length = 12;
        length += Measure<f32>::measure(quat.x);
        length += Measure<f32>::measure(quat.y);
        length += Measure<f32>::measure(quat.z);
        length += Measure<f32>::measure(quat.w);
        return length;
    }
};
template<>
struct Measure<Math::Mat4> {
    static u64 measure(const Math::Mat4& mat) {
        u64 length = 6;
        for(u64 i = 0; i < 4; i++) {
            length += 1;
            for(u64 j = 0; j < 4; j++) {
                length += Measure<f32>::measure(mat[i][j]);
                if(j + 1 < 4) length += 2;
            }
            length += 1;
            if(i + 1 < 4) length += 2;
        }
        return length;
    }
};
template<>
struct Measure<Math::BBox> {
    static u64 measure(const Math::BBox& bbox) {
        u64 length = 20;
        length += Measure<f32>::measure(bbox.min.x);
        length += Measure<f32>::measure(bbox.min.y);
        length += Measure<f32>::measure(bbox.min.z);
        length += Measure<f32>::measure(bbox.max.x);
        length += Measure<f32>::measure(bbox.max.y);
        length += Measure<f32>::measure(bbox.max.z);
        return length;
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
};

template<Allocator O>
struct Write<O, String_View> {
    static u64 write(String<O>& output, u64 idx, String_View value) {
        return output.write(idx, value);
    }
};
template<Allocator O, Allocator A>
struct Write<O, String<A>> {
    static u64 write(String<O>& output, u64 idx, const String<A>& value) {
        return output.write(idx, value);
    }
};
template<Allocator O, Reflectable T>
struct Write<O, Ref<T>> {
    static u64 write(String<O>& output, u64 idx, const Ref<T>& ref) {
        if(!ref) return output.write(idx, "Ref{null}"_v);
        idx = output.write(idx, "Ref{"_v);
        idx = Write<O, T>::write(output, idx, *ref);
        return output.write(idx, '}');
    }
};
template<Allocator O, Reflectable L, Reflectable R>
struct Write<O, Pair<L, R>> {
    static u64 write(String<O>& output, u64 idx, const Pair<L, R>& pair) {
        idx = output.write(idx, "Pair{"_v);
        idx = Write<O, L>::write(output, idx, pair.first);
        idx = output.write(idx, ", "_v);
        idx = Write<O, R>::write(output, idx, pair.second);
        return output.write(idx, '}');
    }
};
template<Allocator O, Reflectable T>
struct Write<O, Storage<T>> {
    static u64 write(String<O>& output, u64 idx, const Storage<T>& storage) {
        idx = output.write(idx, "Storage<"_v);
        idx = output.write(idx, String_View{Reflect<T>::name});
        return output.write(idx, '>');
    }
};
template<Allocator O, Reflectable T>
struct Write<O, Opt<T>> {
    static u64 write(String<O>& output, u64 idx, const Opt<T>& opt) {
        if(!opt) return output.write(idx, "Opt{None}"_v);
        idx = output.write(idx, "Opt{"_v);
        idx = Write<O, T>::write(output, idx, *opt);
        return output.write(idx, '}');
    }
};
template<Allocator O, Reflectable T, u64 N>
struct Write<O, Array<T, N>> {
    static u64 write(String<O>& output, u64 idx, const Array<T, N>& array) {
        idx = output.write(idx, '[');
        for(u64 i = 0; i < N; i++) {
            idx = Write<O, T>::write(output, idx, array[i]);
            if(i + 1 < N) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, ']');
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Vec<T, A>> {
    static u64 write(String<O>& output, u64 idx, const Vec<T, A>& vec) {
        idx = output.write(idx, "Vec["_v);
        for(u64 i = 0; i < vec.length(); i++) {
            idx = Write<O, T>::write(output, idx, vec[i]);
            if(i + 1 < vec.length()) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, ']');
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Box<T, A>> {
    static u64 write(String<O>& output, u64 idx, const Box<T, A>& box) {
        if(!box) return output.write(idx, "Box{null}"_v);
        idx = output.write(idx, "Box{"_v);
        idx = Write<O, T>::write(output, idx, *box);
        return output.write(idx, '}');
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Rc<T, A>> {
    static u64 write(String<O>& output, u64 idx, const Rc<T, A>& rc) {
        if(!rc) return output.write(idx, "Rc{null}"_v);
        idx = output.write(idx, "Rc["_v);
        idx = Write<O, decltype(rc.references())>::write(output, idx, rc.references());
        idx = output.write(idx, "]{"_v);
        idx = Write<O, T>::write(output, idx, *rc);
        return output.write(idx, '}');
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Arc<T, A>> {
    static u64 write(String<O>& output, u64 idx, const Arc<T, A>& arc) {
        if(!arc) return output.write(idx, "Arc{null}"_v);
        idx = output.write(idx, "Arc["_v);
        idx = Write<O, decltype(arc.references())>::write(output, idx, arc.references());
        idx = output.write(idx, "]{"_v);
        idx = Write<O, T>::write(output, idx, *arc);
        return output.write(idx, '}');
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Stack<T, A>> {
    static u64 write(String<O>& output, u64 idx, const Stack<T, A>& stack) {
        idx = output.write(idx, "Stack["_v);
        u64 n = 0;
        for(const T& item : stack) {
            idx = Write<O, T>::write(output, idx, item);
            if(n + 1 < stack.length()) idx = output.write(idx, ", "_v);
            n++;
        }
        return output.write(idx, ']');
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Queue<T, A>> {
    static u64 write(String<O>& output, u64 idx, const Queue<T, A>& queue) {
        idx = output.write(idx, "Queue["_v);
        u64 n = 0;
        for(const T& item : queue) {
            idx = Write<O, T>::write(output, idx, item);
            if(n + 1 < queue.length()) idx = output.write(idx, ", "_v);
            n++;
        }
        return output.write(idx, ']');
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Heap<T, A>> {
    static u64 write(String<O>& output, u64 idx, const Heap<T, A>& heap) {
        idx = output.write(idx, "Heap["_v);
        u64 n = 0;
        for(const T& item : heap) {
            idx = Write<O, T>::write(output, idx, item);
            if(n + 1 < heap.length()) idx = output.write(idx, ", "_v);
            n++;
        }
        return output.write(idx, ']');
    }
};
template<Allocator O, Reflectable K, Reflectable V, Allocator A>
struct Write<O, Map<K, V, A>> {
    static u64 write(String<O>& output, u64 idx, const Map<K, V, A>& map) {
        idx = output.write(idx, "Map["_v);
        u64 n = 0;
        for(const Pair<K, V>& item : map) {
            idx = output.write(idx, "{"_v);
            idx = Write<O, K>::write(output, idx, item.first);
            idx = output.write(idx, " : "_v);
            idx = Write<O, V>::write(output, idx, item.second);
            idx = output.write(idx, '}');
            if(n + 1 < map.length()) idx = output.write(idx, ", "_v);
            n++;
        }
        return output.write(idx, ']');
    }
};
template<Allocator O, Float F, u64 N>
struct Write<O, Math::Vect<F, N>> {
    static u64 write(String<O>& output, u64 idx, const Math::Vect<F, N>& vect) {
        idx = output.write(idx, "Vec"_v);
        idx = Write<O, u64>::write(output, idx, N);
        idx = output.write(idx, '{');
        for(u64 i = 0; i < N; i++) {
            idx = Write<O, F>::write(output, idx, vect[i]);
            if(i + 1 < N) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, '}');
    }
};
template<Allocator O, Signed_Int I, u64 N>
struct Write<O, Math::Vect<I, N>> {
    static u64 write(String<O>& output, u64 idx, const Math::Vect<I, N>& vect) {
        idx = output.write(idx, "Vec"_v);
        idx = Write<O, u64>::write(output, idx, N);
        idx = output.write(idx, "i{"_v);
        for(u64 i = 0; i < N; i++) {
            idx = Write<O, I>::write(output, idx, vect[i]);
            if(i + 1 < N) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, '}');
    }
};
template<Allocator O, Unsigned_Int I, u64 N>
struct Write<O, Math::Vect<I, N>> {
    static u64 write(String<O>& output, u64 idx, const Math::Vect<I, N>& vect) {
        idx = output.write(idx, "Vec"_v);
        idx = Write<O, u64>::write(output, idx, N);
        idx = output.write(idx, "u{"_v);
        for(u64 i = 0; i < N; i++) {
            idx = Write<O, I>::write(output, idx, vect[i]);
            if(i + 1 < N) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, '}');
    }
};
template<Allocator O>
struct Write<O, Math::Quat> {
    static u64 write(String<O>& output, u64 idx, const Math::Quat& quat) {
        idx = output.write(idx, "Quat{"_v);
        idx = Write<O, f32>::write(output, idx, quat.x);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, quat.y);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, quat.z);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, quat.w);
        return output.write(idx, '}');
    }
};
template<Allocator O>
struct Write<O, Math::Mat4> {
    static u64 write(String<O>& output, u64 idx, const Math::Mat4& mat) {
        idx = output.write(idx, "Mat4{"_v);
        for(u64 i = 0; i < 4; i++) {
            idx = output.write(idx, '{');
            for(u64 j = 0; j < 4; j++) {
                idx = Write<O, f32>::write(output, idx, mat[i][j]);
                if(j + 1 < 4) idx = output.write(idx, ", "_v);
            }
            idx = output.write(idx, '}');
            if(i + 1 < 4) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, '}');
    }
};
template<Allocator O>
struct Write<O, Math::BBox> {
    static u64 write(String<O>& output, u64 idx, const Math::BBox& bbox) {
        idx = output.write(idx, "BBox{{"_v);
        idx = Write<O, f32>::write(output, idx, bbox.min.x);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, bbox.min.y);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, bbox.min.z);
        idx = output.write(idx, "}, {"_v);
        idx = Write<O, f32>::write(output, idx, bbox.max.x);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, bbox.max.y);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, bbox.max.z);
        return output.write(idx, "}}"_v);
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

inline Pair<u64, u64> parse_fmt(String_View fmt) {
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
u64 format_length(String_View fmt, const Ts&... args) {
    auto [fmt_length, n_args] = formatting::parse_fmt(fmt);
    assert(n_args == sizeof...(args));
    return fmt_length + (formatting::Measure<Ts>::measure(args) + ...);
}

template<>
inline u64 format_length(String_View fmt) {
    return formatting::parse_fmt(fmt).first;
}

template<Allocator A, typename... Ts>
    requires(Reflectable<Ts> && ...)
String<A> format(String_View fmt, const Ts&... args) {
    u64 length = format_length(fmt, args...);
    String<A> output{length};
    output.set_length(length);
    u64 idx = formatting::write(fmt, 0, output, 0, args...);
    assert(idx == length);
    return output;
}

} // namespace rpp
