
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

namespace detail {

template<typename T>
struct Is_Int {
    constexpr static bool value = false;
};

template<>
struct Is_Int<u8> {
    constexpr static bool value = true;
};

template<>
struct Is_Int<i8> {
    constexpr static bool value = true;
};

template<>
struct Is_Int<u16> {
    constexpr static bool value = true;
};

template<>
struct Is_Int<i16> {
    constexpr static bool value = true;
};

template<>
struct Is_Int<u32> {
    constexpr static bool value = true;
};

template<>
struct Is_Int<i32> {
    constexpr static bool value = true;
};

template<>
struct Is_Int<u64> {
    constexpr static bool value = true;
};

template<>
struct Is_Int<i64> {
    constexpr static bool value = true;
};

template<typename T>
struct Is_Unsigned_Int {
    constexpr static bool value = false;
};

template<>
struct Is_Unsigned_Int<u8> {
    constexpr static bool value = true;
};

template<>
struct Is_Unsigned_Int<u16> {
    constexpr static bool value = true;
};

template<>
struct Is_Unsigned_Int<u32> {
    constexpr static bool value = true;
};

template<>
struct Is_Unsigned_Int<u64> {
    constexpr static bool value = true;
};

template<typename T>
struct Is_Signed_Int {
    constexpr static bool value = false;
};

template<>
struct Is_Signed_Int<i8> {
    constexpr static bool value = true;
};

template<>
struct Is_Signed_Int<i16> {
    constexpr static bool value = true;
};

template<>
struct Is_Signed_Int<i32> {
    constexpr static bool value = true;
};

template<>
struct Is_Signed_Int<i64> {
    constexpr static bool value = true;
};

template<typename T>
struct Is_Float {
    constexpr static bool value = false;
};

template<>
struct Is_Float<f32> {
    constexpr static bool value = true;
};

template<>
struct Is_Float<f64> {
    constexpr static bool value = true;
};

template<typename T>
struct Is_Const {
    constexpr static bool value = false;
};

template<typename T>
struct Is_Const<const T> {
    constexpr static bool value = true;
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
struct Is_Volatile {
    constexpr static bool value = false;
};

template<typename T>
struct Is_Volatile<volatile T> {
    constexpr static bool value = true;
};

template<typename T>
struct Remove_Volatile {
    using type = T;
};

template<typename T>
struct Remove_Volatile<volatile T> {
    using type = T;
};

template<typename T>
struct Is_Pointer {
    constexpr static bool value = false;
};

template<typename T>
struct Is_Pointer<T*> {
    constexpr static bool value = true;
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
    constexpr static bool value = false;
};

template<typename T>
struct Is_Reference<T&> {
    constexpr static bool value = true;
};

template<typename T>
struct Is_Reference<T&&> {
    constexpr static bool value = true;
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
    constexpr static bool value = false;
};

template<typename T>
struct Is_Lvalue_Reference<T&> {
    constexpr static bool value = true;
};

template<typename T>
struct Is_Rvalue_Reference {
    constexpr static bool value = false;
};

template<typename T>
struct Is_Rvalue_Reference<T&&> {
    constexpr static bool value = true;
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
struct Decay<volatile T> {
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
    constexpr static bool value = false;
};

template<typename T>
struct Is_Same<T, T> {
    constexpr static bool value = true;
};

template<typename... Ts>
struct All_Same;

template<typename L, typename R, typename... Ts>
struct All_Same<L, R, Ts...> {
    constexpr static bool value = Is_Same<L, R>::value && All_Same<R, Ts...>::value;
};

template<typename T>
struct All_Same<T> {
    constexpr static bool value = true;
};

template<>
struct All_Same<> {
    constexpr static bool value = true;
};

template<typename Q, typename... Ts>
struct All_Are;

template<typename Q, typename T, typename... Ts>
struct All_Are<Q, T, Ts...> {
    constexpr static bool value = Is_Same<Q, T>::value && All_Are<Q, Ts...>::value;
};

template<typename Q, typename T>
struct All_Are<Q, T> {
    constexpr static bool value = Is_Same<Q, T>::value;
};

template<typename Q>
struct All_Are<Q> {
    constexpr static bool value = true;
};

template<typename Q, typename... Ts>
struct One_Is;

template<typename Q, typename T, typename... Ts>
struct One_Is<Q, T, Ts...> {
    constexpr static bool value = Is_Same<Q, T>::value || One_Is<Q, Ts...>::value;
};

template<typename Q, typename T>
struct One_Is<Q, T> {
    constexpr static bool value = Is_Same<Q, T>::value;
};

template<typename Q>
struct One_Is<Q> {
    constexpr static bool value = false;
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
    constexpr static u64 value = 0;
};

template<typename T, typename H, typename... Ts>
struct Index_Of<T, H, Ts...> {
    constexpr static u64 value = 1 + Index_Of<T, Ts...>::value;
};

template<typename... Ts>
struct Distinct;

template<typename L, typename R, typename... Ts>
struct Distinct<L, R, Ts...> {
    // NOTE(max): quadratic
    constexpr static bool value = !One_Is<L, R, Ts...>::value && Distinct<R, Ts...>::value;
};

template<typename T>
struct Distinct<T> {
    constexpr static bool value = true;
};

template<>
struct Distinct<> {
    constexpr static bool value = true;
};

template<typename T>
constexpr T&& declval() noexcept {
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
    constexpr static bool value = requires(F&& f, Args&&... args) {
        { rpp::detail::forward<F>(f)(rpp::detail::forward<Args>(args)...) };
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
concept Volatile = detail::Is_Volatile<T>::value;

template<typename T>
concept Pointer = detail::Is_Pointer<T>::value;

template<typename T>
concept Reference = detail::Is_Reference<T>::value;

template<typename T>
concept Lvalue_Reference = detail::Is_Lvalue_Reference<T>::value;

template<typename T>
concept Rvalue_Reference = detail::Is_Rvalue_Reference<T>::value;

template<typename T>
concept Not_Void = !detail::Is_Same<typename detail::Decay<T>::type, void>::value;

template<typename T>
concept Not_Function = detail::Is_Const<const T>::value || detail::Is_Reference<T>::value;

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

template<typename D, typename B>
concept Base_Of = __is_base_of(B, D);

template<typename B, typename D>
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
concept Trivially_Constructable = __is_trivially_constructible(T);

template<typename T>
concept Trivially_Copyable = __is_trivially_copyable(T);

template<typename T>
concept Trivially_Movable = __is_trivially_constructible(T, T);

template<typename T>
concept Trivially_Destructible = __is_trivially_destructible(T);

template<typename T>
concept Must_Destruct = !__is_trivially_destructible(T);

template<typename T>
concept Trivial = Trivially_Constructable<T> && Trivially_Copyable<T> && Trivially_Movable<T> &&
                  Trivially_Destructible<T>;

template<typename E>
using Underlying = __underlying_type(E);

template<u64... Is>
using Index_Sequence = detail::Index_Sequence<u64, Is...>;

template<u64 N>
using Make_Index_Sequence = __make_integer_seq<detail::Index_Sequence, u64, N>;

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

template<typename F, typename X>
concept Predicate = requires() { Same<bool, decltype(F::template value<X>)>; };

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

template<typename T>
using With_Lvalue_Ref = T&;

template<typename T>
using With_Rvalue_Ref = T&&;

template<typename T>
using Without_Const = typename detail::Remove_Const<T>::type;

template<typename T>
using Without_Volatile = typename detail::Remove_Volatile<T>::type;

template<typename T = void>
struct Empty {};

template<typename T, T N>
struct Constant {
    constexpr static T value = N;
};

struct Literal {
    constexpr static u64 max_len = 32;

    constexpr Literal() = default;

    template<size_t N>
    constexpr Literal(const char (&literal)[N]) noexcept {
        static_assert(N <= max_len);
        for(u64 i = 0; i < N; i++) {
            c_string[i] = literal[i];
        }
    }

    constexpr bool operator==(const Literal& other) const noexcept {
        for(u64 i = 0; i < max_len; i++) {
            if(c_string[i] != other.c_string[i]) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr operator const char*() const noexcept {
        return c_string;
    }
    char c_string[max_len] = {};
};

template<Move_Constructable T>
constexpr void swap(T& a, T& b) noexcept {
    if constexpr(Trivially_Movable<T>) {
        alignas(alignof(T)) u8 tmp[sizeof(T)];
        Libc::memcpy(tmp, &a, sizeof(T));
        Libc::memcpy(&a, &b, sizeof(T));
        Libc::memcpy(&b, tmp, sizeof(T));
    } else {
        T tmp = rpp::move(a);
        a = rpp::move(b);
        b = rpp::move(tmp);
    }
}

[[nodiscard]] constexpr bool is_constexpr() noexcept {
    return __builtin_is_constant_evaluated();
}

template<class T>
[[nodiscard]] RPP_FORCE_INLINE constexpr T* launder(T* p) noexcept
    requires Not_Function<T> && Not_Void<T>
{
    return __builtin_launder(p);
}

} // namespace rpp
