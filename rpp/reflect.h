
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

#define RPP_OFFSETOF(s, m) __builtin_offsetof(s, m)

#define CASE(case_) Case<T, T::case_, #case_>
#define FIELD(field) Field<decltype(T::field), RPP_OFFSETOF(T, field), #field>

namespace rpp {

template<typename T>
struct Empty {};

namespace detail {
struct Literal {
    static constexpr u64 max_len = 24;
    template<size_t N>
    constexpr Literal(const char (&literal)[N]) {
        static_assert(N <= max_len);
        for(u64 i = 0; i < N; i++) {
            c_string[i] = literal[i];
        }
    }

    operator const char*() const {
        return c_string;
    }
    char c_string[max_len] = {};
};

namespace list {

struct Nil {};

template<typename H, typename T>
struct Cons {
    using Head = H;
    using Tail = T;
};

template<typename T>
struct Is_List {
    static constexpr bool value = false;
};

template<>
struct Is_List<Nil> {
    static constexpr bool value = true;
};

template<typename Head, typename Tail>
struct Is_List<Cons<Head, Tail>> {
    static constexpr bool value = Is_List<Tail>::value;
};

template<typename T>
concept List = Is_List<T>::value;

template<typename... Ts>
struct Make;

template<typename T, typename... Ts>
struct Make<T, Ts...> {
    using L = Cons<T, typename Make<Ts...>::L>;
};

template<>
struct Make<> {
    using L = Nil;
};

template<typename F, typename X>
concept Predicate = requires() { Same<bool, decltype(F::template value<X>)>; };

template<typename F, typename X>
concept Apply = requires(F f) {
    { f.template apply<X>() } -> Same<void>;
};

template<typename F, List L>
struct All;

template<typename F>
struct All<F, Nil> {
    static constexpr bool value = true;
};

template<typename F, typename Head, List Tail>
    requires Predicate<F, Head>
struct All<F, Cons<Head, Tail>> {
    static constexpr bool value = F::template value<Head> && All<F, Tail>::value;
};

template<typename F, List L>
struct Iter;

template<typename F>
struct Iter<F, Nil> {
    template<typename G>
    static void apply(G&& f) {
    }
};

template<typename F, typename Head, List Tail>
    requires Apply<F, Head>
struct Iter<F, Cons<Head, Tail>> {
    template<typename G>
    static void apply(G&& f) {
        f.template apply<Head>();
        Iter<F, Tail>::apply(forward<G>(f));
    }
};

template<List L>
struct Length;

template<>
struct Length<Nil> {
    static constexpr u64 value = 0;
};

template<typename Head, List Tail>
struct Length<Cons<Head, Tail>> {
    static constexpr u64 value = 1 + Length<Tail>::value;
};

template<List L, List Acc>
struct Reverse_Acc;

template<List Acc>
struct Reverse_Acc<Nil, Acc> {
    using type = Acc;
};

template<typename Head, List Tail, List Acc>
struct Reverse_Acc<Cons<Head, Tail>, Acc> {
    using type = typename Reverse_Acc<Tail, Cons<Head, Acc>>::type;
};

template<List L>
using Reversed = typename Reverse_Acc<L, Nil>::type;

} // namespace list

template<typename... Ts>
using List = typename list::Make<Ts...>::L;

template<list::List L>
constexpr u64 List_Length = list::Length<L>::value;

template<typename T>
concept Type_List = list::List<T>;

template<typename P, typename L>
concept Type_List_All = list::All<P, L>::value;

template<u64 N>
struct Constant {
    static constexpr u64 value = N;
};

template<typename... Ts>
struct Index_List_R;

template<>
struct Index_List_R<> {
    using type = list::Nil;
};

template<typename T, typename... Ts>
struct Index_List_R<T, Ts...> {
    using type = list::Cons<Constant<sizeof...(Ts)>, typename Index_List_R<Ts...>::type>;
};

template<typename... Ts>
using Index_List = list::Reversed<typename Index_List_R<Ts...>::type>;

enum class Kind : u8 {
    void_,
    char_,
    i8_,
    i16_,
    i32_,
    i64_,
    u8_,
    u16_,
    u32_,
    u64_,
    f32_,
    f64_,
    bool_,
    array_,
    pointer_,
    enum_,
    record_
};

template<typename T>
struct Reflect;

template<typename T>
concept Reflectable = requires() {
    Same<Literal, decltype(Reflect<T>::name)>;
    Same<Kind, decltype(Reflect<T>::kind)>;
};

template<typename E, E V, Literal N>
struct Case {
    using type = E;
    static constexpr Literal name = N;
    static constexpr E value = V;
};

template<typename F, u64 O, Literal N>
struct Field {
    using type = F;
    static constexpr Literal name = N;
    static constexpr u64 offset = O;
};

template<typename EC>
concept Enum_Case = requires() {
    Reflectable<typename EC::type>;
    { EC::name } -> Same<Literal>;
    { EC::value } -> Same<typename EC::type>;
};

struct Is_Enum_Case {
    template<typename EC>
    static constexpr bool value = Enum_Case<EC>;
};

template<typename RF>
concept Record_Field = requires() {
    Reflectable<typename RF::type>;
    { RF::name } -> Same<Literal>;
    { RF::offset } -> Same<u64>;
};

struct Is_Record_Field {
    template<typename RF>
    static constexpr bool value = Record_Field<RF>;
};

template<typename E>
concept Enum = requires() {
    Reflectable<E>;
    Reflectable<typename Reflect<E>::underlying>;
    Type_List_All<Is_Enum_Case, typename Reflect<E>::members>;
    Same<E, decltype(Reflect<E>::default_)>;
    Reflect<E>::kind == Kind::enum_;
};

template<typename R>
concept Record = requires() {
    Reflectable<R>;
    Type_List_All<Is_Record_Field, typename Reflect<R>::members>;
    Reflect<R>::kind == Kind::record_;
};

template<typename F>
struct Invoke_Case {
    template<typename C>
        requires Invocable<F, const Literal&, typename C::type>
    void apply() {
        f(C::name, C::value);
    }
    F f;
};

template<Enum E, typename F>
    requires Invocable<F, const Literal&, E>
void iterate_enum(F&& f) {
    list::Iter<Invoke_Case<F>, typename Reflect<E>::members>::apply(Invoke_Case<F>{forward<F>(f)});
}

template<typename F, typename T>
concept Field_Iterator = requires(F f, const Literal& name, T& field) {
    { f.template apply<T>(name, field) } -> Same<void>;
};

template<typename F, bool C>
struct Invoke_Field {
    template<typename RF>
        requires Field_Iterator<F, typename RF::type>
    void apply() {
        using Ptr = If<C, const typename RF::type*, typename RF::type*>::type;
        Ptr field = reinterpret_cast<Ptr>(address + RF::offset);
        f.template apply<decltype(*field)>(RF::name, *field);
    }
    F f;
    typename If<C, const u8*, u8*>::type address;
};

template<Record R, typename F>
void iterate_record(F&& f, R& record) {
    u8* address = reinterpret_cast<u8*>(&record);
    list::Iter<Invoke_Field<F, false>, typename Reflect<R>::members>::apply(
        Invoke_Field<F, false>{forward<F>(f), address});
}

template<Record R, typename F>
void iterate_record(F&& f, const R& record) {
    const u8* address = reinterpret_cast<const u8*>(&record);
    list::Iter<Invoke_Field<F, true>, typename Reflect<R>::members>::apply(
        Invoke_Field<F, true>{forward<F>(f), address});
}

template<typename T>
struct Reflect<const T> : Reflect<T> {};

template<typename T>
struct Reflect<T&> : Reflect<T> {};

template<typename T>
struct Reflect<const T&> : Reflect<T> {};

template<typename T>
struct Reflect<T&&> : Reflect<T> {};

template<typename T>
struct Reflect<T*> {
    using underlying = T;
    static constexpr Literal name = Reflect<T>::name;
    static constexpr Kind kind = Kind::pointer_;
};

template<typename T>
struct Reflect<const T*> {
    using underlying = T;
    static constexpr Literal name = Reflect<T>::name;
    static constexpr Kind kind = Kind::pointer_;
};

template<typename T, u64 N>
struct Reflect<T[N]> {
    using underlying = T;
    static constexpr Literal name = Reflect<T>::name;
    static constexpr u64 length = N;
    static constexpr Kind kind = Kind::array_;
};

template<typename T, u64 N>
struct Reflect<const T[N]> {
    using underlying = T;
    static constexpr Literal name = Reflect<T>::name;
    static constexpr u64 length = N;
    static constexpr Kind kind = Kind::array_;
};

template<>
struct Reflect<void> {
    static constexpr Literal name = "void";
    static constexpr Kind kind = Kind::void_;
};

template<>
struct Reflect<decltype(null)> {
    using underlying = void;
    static constexpr Literal name = "null";
    static constexpr Kind kind = Kind::pointer_;
};

template<>
struct Reflect<char> {
    static constexpr Literal name = "char";
    static constexpr Kind kind = Kind::char_;
};

template<>
struct Reflect<i8> {
    static constexpr Literal name = "i8";
    static constexpr Kind kind = Kind::i8_;
};

template<>
struct Reflect<u8> {
    static constexpr Literal name = "u8";
    static constexpr Kind kind = Kind::u8_;
};

template<>
struct Reflect<i16> {
    static constexpr Literal name = "i16";
    static constexpr Kind kind = Kind::i16_;
};

template<>
struct Reflect<u16> {
    static constexpr Literal name = "u16";
    static constexpr Kind kind = Kind::u16_;
};
template<>
struct Reflect<i32> {
    static constexpr Literal name = "i32";
    static constexpr Kind kind = Kind::i32_;
};

template<>
struct Reflect<u32> {
    static constexpr Literal name = "u32";
    static constexpr Kind kind = Kind::u32_;
};

template<>
struct Reflect<i64> {
    static constexpr Literal name = "i64";
    static constexpr Kind kind = Kind::i64_;
};

template<>
struct Reflect<u64> {
    static constexpr Literal name = "u64";
    static constexpr Kind kind = Kind::u64_;
};

template<>
struct Reflect<f32> {
    static constexpr Literal name = "f32";
    static constexpr Kind kind = Kind::f32_;
};

template<>
struct Reflect<f64> {
    static constexpr Literal name = "f64";
    static constexpr Kind kind = Kind::f64_;
};

template<>
struct Reflect<bool> {
    static constexpr Literal name = "bool";
    static constexpr Kind kind = Kind::bool_;
};

template<>
struct Reflect<Kind> {
    using T = Kind;
    using underlying = u8;
    static constexpr Literal name = "Kind";
    static constexpr Kind kind = Kind::enum_;
    static constexpr Kind default_ = Kind::void_;
    using members = List<CASE(void_), CASE(i8_), CASE(i16_), CASE(i32_), CASE(i64_), CASE(u8_),
                         CASE(u16_), CASE(u32_), CASE(u64_), CASE(f32_), CASE(f64_), CASE(bool_),
                         CASE(array_), CASE(pointer_), CASE(enum_), CASE(record_)>;
    static_assert(Enum<T>);
};

} // namespace detail

using detail::Enum;
using detail::Index_List;
using detail::Kind;
using detail::List;
using detail::List_Length;
using detail::Literal;
using detail::Record;
using detail::Reflect;
using detail::Reflectable;
using detail::Type_List;
using detail::Type_List_All;

using detail::iterate_enum;
using detail::iterate_record;

} // namespace rpp
