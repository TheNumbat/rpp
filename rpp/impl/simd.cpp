
#include "../simd.h"

namespace rpp::SIMD {

static_assert(sizeof(F32x4) == 16);
static_assert(alignof(F32x4) == 16);
static_assert(sizeof(F32x8) == 32);
static_assert(alignof(F32x8) == 32);

using float4 = f32 __attribute__((ext_vector_type(4)));
using float8 = f32 __attribute__((ext_vector_type(8)));

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::set1(f32 v) noexcept {
    return {(floatT)(v)};
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::zero() noexcept {
    return set1(0);
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::one() noexcept {
    return set1(1);
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::add(F32x<T> a, F32x<T> b) noexcept {
    return {a.data + b.data};
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::sub(F32x<T> a, F32x<T> b) noexcept {
    return {a.data - b.data};
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::mul(F32x<T> a, F32x<T> b) noexcept {
    return {a.data * b.data};
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::div(F32x<T> a, F32x<T> b) noexcept {
    return {a.data / b.data};
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::min(F32x<T> a, F32x<T> b) noexcept {
    return {__builtin_elementwise_min(a.data, b.data)};
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::max(F32x<T> a, F32x<T> b) noexcept {
    return {__builtin_elementwise_max(a.data, b.data)};
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::floor(F32x<T> a) noexcept {
    return {__builtin_elementwise_floor(a.data)};
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::ceil(F32x<T> a) noexcept {
    return {__builtin_elementwise_ceil(a.data)};
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::abs(F32x<T> a) noexcept {
    return {__builtin_elementwise_abs(a.data)};
}

template<i32 T>
[[nodiscard]] f32 F32x<T>::dp(F32x<T> a, F32x<T> b) noexcept {
    F32x<T> m = mul(a, b);
    // No built-in to reduce float vectors, lets just do it manually
    f32 ret = 0.f;
    for(i32 i = 0; i < T; i++) {
        ret += m.data[i];
    }
    return ret;
}

template<i32 T>
[[nodiscard]] i32 F32x<T>::cmpeq(F32x<T> a, F32x<T> b) noexcept {
    // element-wise comparison produces -1 (all 1's) when equal, 0 otherwise
    // reducing via & to get equality of all members
    return __builtin_reduce_and(a.data == b.data);
}

// explicit template instantiation
template struct F32x<4>;
template struct F32x<8>;

} // namespace rpp::SIMD
