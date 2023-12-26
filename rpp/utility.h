
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

namespace detail {

template<typename T>
struct Is_Int {
    static constexpr bool value = false;
};

template<>
struct Is_Int<u8> {
    static constexpr bool value = true;
};

template<>
struct Is_Int<i8> {
    static constexpr bool value = true;
};

template<>
struct Is_Int<u16> {
    static constexpr bool value = true;
};

template<>
struct Is_Int<i16> {
    static constexpr bool value = true;
};

template<>
struct Is_Int<u32> {
    static constexpr bool value = true;
};

template<>
struct Is_Int<i32> {
    static constexpr bool value = true;
};

template<>
struct Is_Int<u64> {
    static constexpr bool value = true;
};

template<>
struct Is_Int<i64> {
    static constexpr bool value = true;
};

template<typename T>
struct Is_Unsigned_Int {
    static constexpr bool value = false;
};

template<>
struct Is_Unsigned_Int<u8> {
    static constexpr bool value = true;
};

template<>
struct Is_Unsigned_Int<u16> {
    static constexpr bool value = true;
};

template<>
struct Is_Unsigned_Int<u32> {
    static constexpr bool value = true;
};

template<>
struct Is_Unsigned_Int<u64> {
    static constexpr bool value = true;
};

template<typename T>
struct Is_Signed_Int {
    static constexpr bool value = false;
};

template<>
struct Is_Signed_Int<i8> {
    static constexpr bool value = true;
};

template<>
struct Is_Signed_Int<i16> {
    static constexpr bool value = true;
};

template<>
struct Is_Signed_Int<i32> {
    static constexpr bool value = true;
};

template<>
struct Is_Signed_Int<i64> {
    static constexpr bool value = true;
};

template<typename T>
struct Is_Float {
    static constexpr bool value = false;
};

template<>
struct Is_Float<f32> {
    static constexpr bool value = true;
};

template<>
struct Is_Float<f64> {
    static constexpr bool value = true;
};

template<typename T>
struct Is_Const {
    static constexpr bool value = false;
};

template<typename T>
struct Is_Const<const T> {
    static constexpr bool value = true;
};

template<typename T>
struct Add_Const {
    using type = const T;
};

template<typename T>
struct Remove_Const {
    using type = T;
};

template<typename T>
struct Remove_Const<const T> {
    using type = T;
};

template<typename T>
struct Is_Pointer {
    static constexpr bool value = false;
};

template<typename T>
struct Is_Pointer<T*> {
    static constexpr bool value = true;
};

template<typename T>
struct Add_Pointer {
    using type = T*;
};

template<typename T>
struct Remove_Pointer {
    using type = T;
};

template<typename T>
struct Remove_Pointer<T*> {
    using type = T;
};

template<typename T>
struct Is_Reference {
    static constexpr bool value = false;
};

template<typename T>
struct Is_Reference<T&> {
    static constexpr bool value = true;
};

template<typename T>
struct Is_Reference<T&&> {
    static constexpr bool value = true;
};

template<typename T>
struct Remove_Reference {
    using type = T;
};

template<typename T>
struct Remove_Reference<T&> {
    using type = T;
};

template<typename T>
struct Remove_Reference<T&&> {
    using type = T;
};

template<typename T>
struct Is_Lvalue_Reference {
    static constexpr bool value = false;
};

template<typename T>
struct Is_Lvalue_Reference<T&> {
    static constexpr bool value = true;
};

template<typename T>
struct Add_Lvalue_Reference {
    using type = T&;
};

template<typename T>
struct Is_Rvalue_Reference {
    static constexpr bool value = false;
};

template<typename T>
struct Is_Rvalue_Reference<T&&> {
    static constexpr bool value = true;
};

template<typename T>
struct Add_Rvalue_Reference {
    using type = T&&;
};

template<typename T>
struct Decay {
    using type = T;
};

template<typename T>
struct Decay<const T> {
    using type = typename Decay<T>::type;
};

template<typename T>
struct Decay<T&> {
    using type = typename Decay<T>::type;
};

template<typename T>
struct Decay<T&&> {
    using type = typename Decay<T>::type;
};

template<typename T>
constexpr bool True = true;

template<typename T>
constexpr bool False = false;

template<typename L, typename R>
struct Is_Same {
    static constexpr bool value = false;
};

template<typename T>
struct Is_Same<T, T> {
    static constexpr bool value = true;
};

template<typename... Ts>
struct All_Same;

template<typename L, typename R, typename... Ts>
struct All_Same<L, R, Ts...> {
    static constexpr bool value = Is_Same<L, R>::value && All_Same<R, Ts...>::value;
};

template<typename T>
struct All_Same<T> {
    static constexpr bool value = true;
};

template<>
struct All_Same<> {
    static constexpr bool value = true;
};

template<typename Q, typename... Ts>
struct All_Are;

template<typename Q, typename T, typename... Ts>
struct All_Are<Q, T, Ts...> {
    static constexpr bool value = Is_Same<Q, T>::value && All_Are<Q, Ts...>::value;
};

template<typename Q, typename T>
struct All_Are<Q, T> {
    static constexpr bool value = Is_Same<Q, T>::value;
};

template<typename Q>
struct All_Are<Q> {
    static constexpr bool value = true;
};

template<typename Q, typename... Ts>
struct One_Is;

template<typename Q, typename T, typename... Ts>
struct One_Is<Q, T, Ts...> {
    static constexpr bool value = Is_Same<Q, T>::value || One_Is<Q, Ts...>::value;
};

template<typename Q, typename T>
struct One_Is<Q, T> {
    static constexpr bool value = Is_Same<Q, T>::value;
};

template<typename Q>
struct One_Is<Q> {
    static constexpr bool value = false;
};

template<bool b, typename T, typename F>
struct If;

template<typename T, typename F>
struct If<true, T, F> {
    using type = T;
};

template<typename T, typename F>
struct If<false, T, F> {
    using type = F;
};

template<u64 I, typename... Ts>
struct Choose;

template<typename T, typename... Ts>
struct Choose<0, T, Ts...> {
    using type = T;
};

template<u64 I, typename T, typename... Ts>
struct Choose<I, T, Ts...> {
    using type = typename Choose<I - 1, Ts...>::type;
};

template<typename T, typename... Ts>
    requires One_Is<T, Ts...>::value
struct Index_Of;

template<typename T, typename... Ts>
struct Index_Of<T, T, Ts...> {
    static constexpr u64 value = 0;
};

template<typename T, typename H, typename... Ts>
struct Index_Of<T, H, Ts...> {
    static constexpr u64 value = 1 + Index_Of<T, Ts...>::value;
};

template<typename... Ts>
struct Distinct;

template<typename L, typename R, typename... Ts>
struct Distinct<L, R, Ts...> {
    // NOTE(max): quadratic
    static constexpr bool value = !One_Is<L, R, Ts...>::value && Distinct<R, Ts...>::value;
};

template<typename T>
struct Distinct<T> {
    static constexpr bool value = true;
};

template<>
struct Distinct<> {
    static constexpr bool value = true;
};

template<typename T>
constexpr typename Add_Rvalue_Reference<T>::type declval() noexcept {
    static_assert(False<T>, "Declval not allowed in an evaluated context.");
}

template<typename T>
RPP_MSVC_INTRINSIC RPP_FORCE_INLINE constexpr typename Remove_Reference<T>::type&&
move(T&& value) noexcept {
    return static_cast<typename Remove_Reference<T>::type&&>(value);
}

template<typename T>
RPP_MSVC_INTRINSIC RPP_FORCE_INLINE constexpr T&&
forward(typename Remove_Reference<T>::type& value) noexcept {
    return static_cast<T&&>(value);
}

template<typename T>
RPP_MSVC_INTRINSIC RPP_FORCE_INLINE constexpr T&&
forward(typename Remove_Reference<T>::type&& value) noexcept {
    static_assert(!Is_Lvalue_Reference<T>::value, "Bad forward call.");
    return static_cast<T&&>(value);
}

template<typename F, typename... Args>
struct Is_Invocable {
    static constexpr bool value = requires(F&& f, Args&&... args) {
        { forward<F>(f)(forward<Args>(args)...) };
    };
};

template<typename F, typename... Args>
struct Return_Type {
    using type = decltype(declval<typename Decay<F>::type>()(declval<Args>()...));
};

template<typename I, I... Is>
    requires Is_Int<I>::value
struct Index_Sequence {
    using value_type = I;
};

} // namespace detail

using detail::forward;
using detail::move;

template<typename I>
concept Int = detail::Is_Int<I>::value;

template<typename I>
concept Unsigned_Int = detail::Is_Unsigned_Int<I>::value;

template<typename I>
concept Signed_Int = detail::Is_Signed_Int<I>::value;

template<typename T>
concept Float = detail::Is_Float<T>::value;

template<typename T>
concept Const = detail::Is_Const<T>::value;

template<typename T>
concept Pointer = detail::Is_Pointer<T>::value;

template<typename T>
concept Reference = detail::Is_Reference<T>::value;

template<typename T>
concept Lvalue_Reference = detail::Is_Lvalue_Reference<T>::value;

template<typename T>
concept Rvalue_Reference = detail::Is_Rvalue_Reference<T>::value;

template<typename L, typename R>
concept Same = detail::Is_Same<L, R>::value;

template<typename... Ts>
concept All_Same = detail::All_Same<Ts...>::value;

template<typename Q, typename... Ts>
concept All_Are = detail::All_Are<Q, Ts...>::value;

template<typename Q, typename... Ts>
concept One_Is = detail::One_Is<Q, Ts...>::value;

template<typename... Ts>
concept Distinct = detail::Distinct<Ts...>::value;

template<typename F, typename... Args>
concept Invocable = detail::Is_Invocable<F, Args...>::value;

template<typename B, typename D>
concept Base_Of = __is_base_of(B, D);

template<typename D, typename B>
concept Derived_From = __is_base_of(B, D);

template<typename T>
concept Abstract = __is_abstract(T);

template<typename T>
concept Not_Abstract = !__is_abstract(T);

template<typename T, typename... Args>
concept Constructable = __is_constructible(T, Args...);

template<typename T>
concept Default_Constructable = __is_constructible(T);

template<typename T>
concept Move_Constructable = __is_constructible(T, T);

template<typename T>
concept Copy_Constructable = __is_constructible(T, const T&);

template<typename T>
concept Trivially_Copyable = __is_trivially_copyable(T);

template<typename T>
concept Trivially_Movable = __is_trivially_constructible(T, T);

template<typename T>
concept Trivially_Destructible = __is_trivially_destructible(T);

template<typename T>
concept Must_Destruct = !__is_trivially_destructible(T);

template<typename T>
concept Trivial = __is_trivially_constructible(T) && __is_trivially_copyable(T);

template<u64... Is>
using Index_Sequence = detail::Index_Sequence<u64, Is...>;

template<u64 N>
using Make_Index_Sequence = __make_integer_seq<detail::Index_Sequence, u64, N>;

template<typename T>
concept Movable = Move_Constructable<T> || Trivial<T>;

template<typename T>
concept Equality = requires(const T& l, const T& r) {
    { l == r } -> Same<bool>;
};

template<typename T>
concept Ordered = requires(const T& l, const T& r) {
    { l < r } -> Same<bool>;
};

template<typename T>
concept Clone = Move_Constructable<T> && requires(const T& value) {
    { value.clone() } -> Same<T>;
};

template<u64 N, typename... Ts>
concept Length = sizeof...(Ts) == N;

template<typename... Ts>
using Index_Sequence_For = Make_Index_Sequence<sizeof...(Ts)>;

template<typename T>
using Decay = typename detail::Decay<T>::type;

template<bool b, typename T, typename F>
using If = typename detail::If<b, T, F>::type;

template<u64 I, typename... Ts>
    requires(I < sizeof...(Ts))
using Choose = typename detail::Choose<I, Ts...>::type;

template<typename T, typename... Ts>
    requires One_Is<T, Ts...>
constexpr u64 Index_Of = detail::Index_Of<T, Ts...>::value;

template<typename F, typename... Args>
    requires Invocable<F, Args...>
using Invoke_Result = detail::Return_Type<F, Args...>::type;

template<typename T = void>
struct Empty {};

template<typename T, T N>
struct Constant {
    static constexpr T value = N;
};

struct Literal {
    static constexpr u64 max_len = 24;

    constexpr Literal() = default;

    template<size_t N>
    constexpr Literal(const char (&literal)[N]) {
        static_assert(N <= max_len);
        for(u64 i = 0; i < N; i++) {
            c_string[i] = literal[i];
        }
    }

    constexpr operator const char*() const {
        return c_string;
    }
    char c_string[max_len] = {};
};

template<Movable T>
void swap(T& a, T& b) {
    if constexpr(Trivially_Movable<T>) {
        alignas(alignof(T)) u8 tmp[sizeof(T)];
        Libc::memcpy(tmp, &a, sizeof(T));
        Libc::memcpy(&a, &b, sizeof(T));
        Libc::memcpy(&b, tmp, sizeof(T));
    } else {
        T tmp = move(a);
        a = move(b);
        b = move(tmp);
    }
}

} // namespace rpp
