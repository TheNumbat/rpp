
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

#define RPP_OFFSETOF(s, m) __builtin_offsetof(s, m)

#define CASE(case_) ::rpp::Reflect::Case<T, T::case_, #case_>
#define FIELD(field) ::rpp::Reflect::Field<decltype(T::field), RPP_OFFSETOF(T, field), #field>

namespace rpp {

namespace Reflect {

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

template<typename F, typename X>
concept Predicate = requires() { Same<bool, decltype(F::template value<X>)>; };

namespace detail {

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

template<typename... Ts>
struct Make;

template<typename T, typename... Ts>
struct Make<T, Ts...> {
    using type = Cons<T, typename Make<Ts...>::type>;
};

template<>
struct Make<> {
    using type = Nil;
};

template<typename F, typename L>
struct All;

template<typename F>
struct All<F, Nil> {
    static constexpr bool value = true;
};

template<typename F, typename Head, typename Tail>
    requires Predicate<F, Head>
struct All<F, Cons<Head, Tail>> {
    static constexpr bool value = F::template value<Head> && All<F, Tail>::value;
};

template<typename F, typename L>
struct Iter;

template<typename F>
struct Iter<F, Nil> {
    template<typename G>
    static constexpr void apply(G&& f) {
    }
};

template<typename F, typename X>
concept Apply = requires(F&& f) {
    { forward<F>(f).template apply<X>() } -> Same<void>;
};

template<typename F, typename Head, typename Tail>
    requires Apply<F, Head>
struct Iter<F, Cons<Head, Tail>> {
    template<typename G>
        requires Same<Decay<F>, Decay<G>>
    static constexpr void apply(G&& f) {
        forward<G>(f).template apply<Head>();
        Iter<F, Tail>::apply(forward<F>(f));
    }
};

template<typename L>
struct Length;

template<>
struct Length<Nil> {
    static constexpr u64 value = 0;
};

template<typename Head, typename Tail>
struct Length<Cons<Head, Tail>> {
    static constexpr u64 value = 1 + Length<Tail>::value;
};

template<typename L, typename Acc>
struct Rerverse;

template<typename Acc>
struct Rerverse<Nil, Acc> {
    using type = Acc;
};

template<typename Head, typename Tail, typename Acc>
struct Rerverse<Cons<Head, Tail>, Acc> {
    using type = typename Rerverse<Tail, Cons<Head, Acc>>::type;
};

template<u64 N, typename... Ts>
struct Enumerated;

template<u64 N>
struct Enumerated<N> {
    using type = Nil;
};

template<u64 N, typename T, typename... Ts>
struct Enumerated<N, T, Ts...> {
    using type = Cons<Constant<u64, N>, typename Enumerated<N + 1, Ts...>::type>;
};

} // namespace detail

template<typename T>
concept Type_List = detail::Is_List<T>::value;

template<typename... Ts>
using List = typename detail::Make<Ts...>::type;

template<Type_List L>
using Reverse = typename detail::Rerverse<L, detail::Nil>::type;

template<typename... Ts>
using Enumerate = typename detail::Enumerated<0, Ts...>::type;

template<Type_List L>
constexpr u64 List_Length = detail::Length<L>::value;

template<typename P, typename L>
concept All = Type_List<L> && detail::All<P, L>::value;

template<typename F, Type_List L>
using Iter = detail::Iter<F, L>;

template<typename T>
struct Refl;

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

template<typename T>
concept Reflectable = requires() {
    Same<Literal, decltype(Refl<T>::name)>;
    Same<Kind, decltype(Refl<T>::kind)>;
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
    Reflectable<typename Refl<E>::underlying>;
    All<Is_Enum_Case, typename Refl<E>::members>;
    Same<E, decltype(Refl<E>::default_)>;
    Refl<E>::kind == Kind::enum_;
};

template<typename R>
concept Record = requires() {
    Reflectable<R>;
    All<Is_Record_Field, typename Refl<R>::members>;
    Refl<R>::kind == Kind::record_;
};

template<typename F>
struct Iter_Case {
    template<typename Case>
    void apply() {
        forward<F>(f)(Case::name, Case::value);
    }
    F&& f;
};

template<Enum E, typename F>
void iterate_enum(F&& f) {
    Iter<Iter_Case<F>, typename Refl<E>::members>::apply(Iter_Case<F>{forward<F>(f)});
}

template<typename F, typename T>
concept Const_Field_Iterator = requires(F f, const Literal& name, const T& field) {
    { f.template apply<T>(name, field) } -> Same<void>;
};

template<typename F, typename T>
concept Field_Iterator = requires(F f, const Literal& name, T& field) {
    { f.template apply<T>(name, field) } -> Same<void>;
};

template<typename F, bool is_const>
struct Iter_Field {
    template<typename Field>
        requires(is_const ? Const_Field_Iterator<F, Decay<typename Field::type>>
                          : Field_Iterator<F, Decay<typename Field::type>>)
    void apply() {
        using Ptr = If<is_const, const typename Field::type*, typename Field::type*>;
        Ptr field = reinterpret_cast<Ptr>(address + Field::offset);
        f.template apply<decltype(*field)>(Field::name, *field);
    }
    F f;
    If<is_const, const u8*, u8*> address;
};

template<Record R, typename F>
void iterate_record(F&& f, R& record) {
    u8* address = reinterpret_cast<u8*>(&record);
    Iter<Iter_Field<F, false>, typename Refl<R>::members>::apply(
        Iter_Field<F, false>{forward<F>(f), address});
}

template<Record R, typename F>
void iterate_record(F&& f, const R& record) {
    const u8* address = reinterpret_cast<const u8*>(&record);
    Iter<Iter_Field<F, true>, typename Refl<R>::members>::apply(
        Iter_Field<F, true>{forward<F>(f), address});
}

template<typename T>
struct Refl<const T> : Refl<T> {};

template<typename T>
struct Refl<T&> : Refl<T> {};

template<typename T>
struct Refl<const T&> : Refl<T> {};

template<typename T>
struct Refl<T&&> : Refl<T> {};

template<typename T>
struct Refl<T*> {
    using underlying = T;
    static constexpr Literal name = Refl<T>::name;
    static constexpr Kind kind = Kind::pointer_;
};

template<typename T>
struct Refl<const T*> {
    using underlying = T;
    static constexpr Literal name = Refl<T>::name;
    static constexpr Kind kind = Kind::pointer_;
};

template<typename T, u64 N>
struct Refl<T[N]> {
    using underlying = T;
    static constexpr Literal name = Refl<T>::name;
    static constexpr u64 length = N;
    static constexpr Kind kind = Kind::array_;
};

template<typename T, u64 N>
struct Refl<const T[N]> {
    using underlying = T;
    static constexpr Literal name = Refl<T>::name;
    static constexpr u64 length = N;
    static constexpr Kind kind = Kind::array_;
};

template<>
struct Refl<void> {
    static constexpr Literal name = "void";
    static constexpr Kind kind = Kind::void_;
};

template<>
struct Refl<decltype(null)> {
    using underlying = void;
    static constexpr Literal name = "null";
    static constexpr Kind kind = Kind::pointer_;
};

template<>
struct Refl<char> {
    static constexpr Literal name = "char";
    static constexpr Kind kind = Kind::char_;
};

template<>
struct Refl<i8> {
    static constexpr Literal name = "i8";
    static constexpr Kind kind = Kind::i8_;
};

template<>
struct Refl<u8> {
    static constexpr Literal name = "u8";
    static constexpr Kind kind = Kind::u8_;
};

template<>
struct Refl<i16> {
    static constexpr Literal name = "i16";
    static constexpr Kind kind = Kind::i16_;
};

template<>
struct Refl<u16> {
    static constexpr Literal name = "u16";
    static constexpr Kind kind = Kind::u16_;
};
template<>
struct Refl<i32> {
    static constexpr Literal name = "i32";
    static constexpr Kind kind = Kind::i32_;
};

template<>
struct Refl<u32> {
    static constexpr Literal name = "u32";
    static constexpr Kind kind = Kind::u32_;
};

template<>
struct Refl<i64> {
    static constexpr Literal name = "i64";
    static constexpr Kind kind = Kind::i64_;
};

template<>
struct Refl<u64> {
    static constexpr Literal name = "u64";
    static constexpr Kind kind = Kind::u64_;
};

template<>
struct Refl<f32> {
    static constexpr Literal name = "f32";
    static constexpr Kind kind = Kind::f32_;
};

template<>
struct Refl<f64> {
    static constexpr Literal name = "f64";
    static constexpr Kind kind = Kind::f64_;
};

template<>
struct Refl<bool> {
    static constexpr Literal name = "bool";
    static constexpr Kind kind = Kind::bool_;
};

template<>
struct Refl<Kind> {
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

} // namespace Reflect

using Reflect::List;
using Reflect::Reflectable;

} // namespace rpp
