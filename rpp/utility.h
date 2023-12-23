
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

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
concept Const = Is_Const<T>::value;

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
concept Pointer = Is_Pointer<T>::value;

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
struct Is_Lvalue_Reference {
    static constexpr bool value = false;
};

template<typename T>
struct Is_Lvalue_Reference<T&> {
    static constexpr bool value = true;
};

template<typename T>
concept Lvalue_Reference = Is_Lvalue_Reference<T>::value;

template<typename T>
struct Is_Rvalue_Reference {
    static constexpr bool value = false;
};

template<typename T>
struct Is_Rvalue_Reference<T&&> {
    static constexpr bool value = true;
};

template<typename T>
concept Rvalue_Reference = Is_Rvalue_Reference<T>::value;

template<typename T>
struct Add_Lvalue_Reference {
    using type = T&;
};

template<typename T>
struct Add_Rvalue_Reference {
    using type = T&&;
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
concept Reference = Is_Reference<T>::value;

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

template<typename L, typename R>
struct Is_Same {
    static constexpr bool value = false;
};
template<typename T>
struct Is_Same<T, T> {
    static constexpr bool value = true;
};

template<typename L, typename R>
concept Same = Is_Same<L, R>::value;

template<typename... Ts>
struct Are_Same;

template<>
struct Are_Same<> {
    static constexpr bool value = true;
};

template<typename T>
struct Are_Same<T> {
    static constexpr bool value = true;
};

template<typename L, typename R, typename... Ts>
struct Are_Same<L, R, Ts...> {
    static constexpr bool value = Is_Same<L, R>::value && Are_Same<R, Ts...>::value;
};

template<typename... Ts>
concept All_Same = Are_Same<Ts...>::value;

template<u64 I, typename... Ts>
    requires(I < sizeof...(Ts))
struct At_Index;

template<typename T, typename... Ts>
struct At_Index<0, T, Ts...> {
    using type = T;
};

template<u64 I, typename T, typename... Ts>
struct At_Index<I, T, Ts...> {
    using type = typename At_Index<I - 1, Ts...>::type;
};

template<u64 I, typename... Ts>
using Index = typename At_Index<I, Ts...>::type;

template<typename F, typename... Args>
using Invoke_Result = std::invoke_result_t<F, Args...>;

template<typename Q, typename T, typename... Ts>
struct All_Are {
    static constexpr bool value = Is_Same<Q, T>::value && All_Are<Q, Ts...>::value;
};
template<typename Q, typename T>
struct All_Are<Q, T> {
    static constexpr bool value = Is_Same<Q, T>::value;
};

template<typename Q, typename... Ts>
concept All = All_Are<Q, Ts...>::value;

template<typename Q, typename T, typename... Ts>
struct One_Is {
    static constexpr bool value = Is_Same<Q, T>::value || One_Is<Q, Ts...>::value;
};
template<typename Q, typename T>
struct One_Is<Q, T> {
    static constexpr bool value = Is_Same<Q, T>::value;
};

template<typename Q, typename... Ts>
concept One = One_Is<Q, Ts...>::value;

template<typename T, typename... Ts>
    requires One<T, Ts...>
struct Index_Of;

template<typename T, typename... Ts>
struct Index_Of<T, T, Ts...> {
    static constexpr u64 value = 0;
};

template<typename T, typename H, typename... Ts>
struct Index_Of<T, H, Ts...> {
    static constexpr u64 value = 1 + Index_Of<T, Ts...>::value;
};

#if defined COMPILER_MSVC || defined COMPILER_CLANG

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

#else

#include <type_traits>

template<typename B, typename D>
concept Base_Of = std::is_base_of_v<B, D>;

template<typename D, typename B>
concept Derived_From = std::is_base_of_v<B, D>;

template<typename T>
concept Abstract = std::is_abstract_v<T>;

template<typename T>
concept Not_Abstract = !std::is_abstract_v<T>;

template<typename T, typename... Args>
concept Constructable = std::is_constructible_v<T, Args...>;

template<typename T>
concept Default_Constructable = std::is_default_constructible_v<T>;

template<typename T>
concept Move_Constructable = std::is_move_constructible_v<T>;

template<typename T>
concept Copy_Constructable = std::is_copy_constructible_v<T>;

template<typename T>
concept Trivially_Copyable = std::is_trivially_copyable_v<T>;

template<typename T>
concept Trivially_Movable = std::is_trivially_move_constructible_v<T>;

template<typename T>
concept Trivially_Destructible = std::is_trivially_destructible_v<T>;

template<typename T>
concept Must_Destruct = !std::is_trivially_destructible_v<T>;

template<typename T>
concept Trivial = std::is_trivial_v<T>;

#endif

template<typename F, typename... Args>
concept Invocable = std::is_invocable_v<F, Args...>;

template<typename T>
struct Empty {};

template<typename... Ts>
struct All_Distinct;

template<>
struct All_Distinct<> {
    static constexpr bool value = true;
};

template<typename T>
struct All_Distinct<T> {
    static constexpr bool value = true;
};

template<typename L, typename R, typename... Ts>
struct All_Distinct<L, R, Ts...> {
    // Quadratic
    static constexpr bool value = !One<L, R, Ts...> && All_Distinct<R, Ts...>::value;
};

template<typename... Ts>
concept Distinct = All_Distinct<Ts...>::value;

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
concept Float = Is_Float<T>::value;

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

template<typename I>
concept Int = Is_Int<I>::value;

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

template<typename I>
concept Unsigned_Int = Is_Unsigned_Int<I>::value;

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

template<typename I>
concept Signed_Int = Is_Signed_Int<I>::value;

// clang-format off

#define DBL_DECIMAL_DIG  17                      // # of decimal digits of rounding precision
#define DBL_DIG          15                      // # of decimal digits of precision
#define DBL_EPSILON      2.2204460492503131e-016 // smallest such that 1.0+DBL_EPSILON != 1.0
#define DBL_MANT_DIG     53                      // # of bits in mantissa
#define DBL_MAX          1.7976931348623158e+308 // max value
#define DBL_MAX_10_EXP   308                     // max decimal exponent
#define DBL_MAX_EXP      1024                    // max binary exponent
#define DBL_MIN          2.2250738585072014e-308 // min positive value
#define DBL_MIN_10_EXP   (-307)                  // min decimal exponent
#define DBL_MIN_EXP      (-1021)                 // min binary exponent
#define DBL_TRUE_MIN     4.9406564584124654e-324 // min positive value

#define FLT_DECIMAL_DIG  9                       // # of decimal digits of rounding precision
#define FLT_DIG          6                       // # of decimal digits of precision
#define FLT_EPSILON      1.192092896e-07F        // smallest such that 1.0+FLT_EPSILON != 1.0
#define FLT_HAS_SUBNORM  1                       // type does support subnormal numbers
#define FLT_MANT_DIG     24                      // # of bits in mantissa
#define FLT_MAX          3.402823466e+38F        // max value
#define FLT_MAX_10_EXP   38                      // max decimal exponent
#define FLT_MAX_EXP      128                     // max binary exponent
#define FLT_MIN          1.175494351e-38F        // min normalized positive value
#define FLT_MIN_10_EXP   (-37)                   // min decimal exponent
#define FLT_MIN_EXP      (-125)                  // min binary exponent
#define FLT_TRUE_MIN     1.401298464e-45F        // min positive value

// clang-format on

template<typename T>
struct Limits;

template<>
struct Limits<u8> {
    static constexpr u8 max() {
        return UINT8_MAX;
    }
    static constexpr u8 min() {
        return 0;
    }
};

template<>
struct Limits<u16> {
    static constexpr u16 max() {
        return UINT16_MAX;
    }
    static constexpr u16 min() {
        return 0;
    }
};

template<>
struct Limits<u32> {
    static constexpr u32 max() {
        return UINT32_MAX;
    }
    static constexpr u32 min() {
        return 0;
    }
};

template<>
struct Limits<u64> {
    static constexpr u64 max() {
        return UINT64_MAX;
    }
    static constexpr u64 min() {
        return 0;
    }
};

template<>
struct Limits<i8> {
    static constexpr i8 max() {
        return INT8_MAX;
    }
    static constexpr i8 min() {
        return INT8_MIN;
    }
};

template<>
struct Limits<i16> {
    static constexpr i16 max() {
        return INT16_MAX;
    }
    static constexpr i16 min() {
        return INT16_MIN;
    }
};

template<>
struct Limits<i32> {
    static constexpr i32 max() {
        return INT32_MAX;
    }
    static constexpr i32 min() {
        return INT32_MIN;
    }
};

template<>
struct Limits<i64> {
    static constexpr i64 max() {
        return INT64_MAX;
    }
    static constexpr i64 min() {
        return INT64_MIN;
    }
};

template<>
struct Limits<f32> {
    static constexpr f32 max() {
        return FLT_MAX;
    }
    static constexpr f32 min() {
        return -FLT_MAX;
    }
    static constexpr f32 smallest() {
        return FLT_MIN;
    }
    static constexpr f32 epsilon() {
        return FLT_EPSILON;
    }
};

template<>
struct Limits<f64> {
    static constexpr f64 max() {
        return DBL_MAX;
    }
    static constexpr f64 min() {
        return -DBL_MAX;
    }
    static constexpr f64 smallest() {
        return DBL_MIN;
    }
    static constexpr f64 epsilon() {
        return DBL_EPSILON;
    }
};

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

template<typename T>
concept Movable = Move_Constructable<T> || Trivial<T>;

template<u64 N, typename... Ts>
concept Length = sizeof...(Ts) == N;

template<Movable T>
void swap(T& a, T& b) {
    if constexpr(Trivially_Movable<T>) {
        alignas(alignof(T)) u8 tmp[sizeof(T)];
        Libc::memcpy(tmp, &a, sizeof(T));
        Libc::memcpy(&a, &b, sizeof(T));
        Libc::memcpy(&b, tmp, sizeof(T));
    } else {
        T tmp = std::move(a);
        a = std::move(b);
        b = std::move(tmp);
    }
}

} // namespace rpp
