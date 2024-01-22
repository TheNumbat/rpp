
#include "../simd.h"

namespace rpp::SIMD {

static_assert(sizeof(F32x4) == 16);
static_assert(alignof(F32x4) == 16);
static_assert(sizeof(F32x8) == 32);
static_assert(alignof(F32x8) == 32);

using f32_4 = f32 __attribute__((ext_vector_type(4)));
using f32_8 = f32 __attribute__((ext_vector_type(8)));

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::set1(f32 v) noexcept {
    return {(f32_N)(v)};
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::zero() noexcept {
    return set1(0);
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::one() noexcept {
    return set1(1);
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::add(F32x<N> a, F32x<N> b) noexcept {
    return {a.data + b.data};
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::sub(F32x<N> a, F32x<N> b) noexcept {
    return {a.data - b.data};
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::mul(F32x<N> a, F32x<N> b) noexcept {
    return {a.data * b.data};
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::div(F32x<N> a, F32x<N> b) noexcept {
    return {a.data / b.data};
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::min(F32x<N> a, F32x<N> b) noexcept {
    return {__builtin_elementwise_min(a.data, b.data)};
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::max(F32x<N> a, F32x<N> b) noexcept {
    return {__builtin_elementwise_max(a.data, b.data)};
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::floor(F32x<N> a) noexcept {
    return {__builtin_elementwise_floor(a.data)};
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::ceil(F32x<N> a) noexcept {
    return {__builtin_elementwise_ceil(a.data)};
}

template<i32 N>
[[nodiscard]] F32x<N> F32x<N>::abs(F32x<N> a) noexcept {
    return {__builtin_elementwise_abs(a.data)};
}

template<i32 N>
[[nodiscard]] f32 F32x<N>::dp(F32x<N> a, F32x<N> b) noexcept {
    F32x<N> m = mul(a, b);
    // No built-in to reduce float vectors, lets just do it manually
    f32 ret = 0.f;
    for(i32 i = 0; i < N; i++) {
        ret += m.data[i];
    }
    return ret;
}

template<i32 N>
[[nodiscard]] i32 F32x<N>::cmpeq(F32x<N> a, F32x<N> b) noexcept {
    // element-wise comparison produces -1 (all 1's) when equal, 0 otherwise
    // reducing via & to get equality of all members
    return __builtin_reduce_and(a.data == b.data);
}

// explicit template instantiation
template struct F32x<4>;
template struct F32x<8>;

} // namespace rpp::SIMD
