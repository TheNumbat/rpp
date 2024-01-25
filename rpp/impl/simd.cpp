
#include "../simd.h"

#ifdef RPP_COMPILER_MSVC
#include <immintrin.h>
#endif

namespace rpp::SIMD {

static_assert(sizeof(F32x4) == 16);
static_assert(alignof(F32x4) == 16);
static_assert(sizeof(F32x8) == 32);
static_assert(alignof(F32x8) == 32);

#ifdef RPP_COMPILER_MSVC

#ifndef __AVX2__
#error "Unsupported architecture for MSVC: AVX2 is required".
#endif

[[nodiscard]] static __m128 of(F32x4 a) noexcept {
    return *reinterpret_cast<__m128*>(a.data);
}

[[nodiscard]] static F32x4 to(__m128 a) noexcept {
    return *reinterpret_cast<F32x4*>(&a);
}

[[nodiscard]] static __m256 of(F32x8 a) noexcept {
    return *reinterpret_cast<__m256*>(a.data);
}

[[nodiscard]] static F32x8 to(__m256 a) noexcept {
    return *reinterpret_cast<F32x8*>(&a);
}

[[nodiscard]] F32x4 F32x4::set1(f32 v) noexcept {
    return to(_mm_set1_ps(v));
}

[[nodiscard]] F32x4 F32x4::set(f32 x, f32 y, f32 z, f32 w) noexcept {
    // reversed to preserve big-endianness
    return to(_mm_set_ps(w, z, y, x));
}

[[nodiscard]] F32x4 F32x4::zero() noexcept {
    return to(_mm_setzero_ps());
}

[[nodiscard]] F32x4 F32x4::one() noexcept {
    return to(_mm_setzero_ps());
}

[[nodiscard]] F32x4 F32x4::add(F32x4 a, F32x4 b) noexcept {
    return to(_mm_add_ps(of(a), of(b)));
}

[[nodiscard]] F32x4 F32x4::sub(F32x4 a, F32x4 b) noexcept {
    return to(_mm_sub_ps(of(a), of(b)));
}

[[nodiscard]] F32x4 F32x4::mul(F32x4 a, F32x4 b) noexcept {
    return to(_mm_mul_ps(of(a), of(b)));
}

[[nodiscard]] F32x4 F32x4::div(F32x4 a, F32x4 b) noexcept {
    return to(_mm_div_ps(of(a), of(b)));
}

[[nodiscard]] F32x4 F32x4::min(F32x4 a, F32x4 b) noexcept {
    return to(_mm_min_ps(of(a), of(b)));
}

[[nodiscard]] F32x4 F32x4::max(F32x4 a, F32x4 b) noexcept {
    return to(_mm_max_ps(of(a), of(b)));
}

[[nodiscard]] F32x4 F32x4::floor(F32x4 a) noexcept {
    return to(_mm_floor_ps(of(a)));
}

[[nodiscard]] F32x4 F32x4::ceil(F32x4 a) noexcept {
    return to(_mm_ceil_ps(of(a)));
}

[[nodiscard]] F32x4 F32x4::abs(F32x4 a) noexcept {
    return to(_mm_andnot_ps(_mm_set1_ps(-0.0f), of(a)));
}

[[nodiscard]] f32 F32x4::dp(F32x4 a, F32x4 b) noexcept {
    return _mm_cvtss_f32(_mm_dp_ps(of(a), of(b), 0xff));
}

[[nodiscard]] i32 F32x4::cmpeq(F32x4 a, F32x4 b) noexcept {
    return to(_mm_movemask_ps(_mm_cmpeq_ps(of(a), of(b)));
}

[[nodiscard]] F32x8 F32x8::set1(f32 v) noexcept {
    return to(_mm256_set1_ps(v));
}

[[nodiscard]] F32x8 F32x8::set(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h) noexcept {
    // reversed to preserve big-endianness
    return to(_mm256_set_ps(h, g, f, e, d, c, b, a));
}

[[nodiscard]] F32x8 F32x8::zero() noexcept {
    return to(_mm256_setzero_ps());
}

[[nodiscard]] F32x8 F32x8::one() noexcept {
    return to(_mm256_setzero_ps());
}

[[nodiscard]] F32x8 F32x8::add(F32x8 a, F32x8 b) noexcept {
    return to(_mm256_add_ps(of(a), of(b)));
}

[[nodiscard]] F32x8 F32x8::sub(F32x8 a, F32x8 b) noexcept {
    return to(_mm256_sub_ps(of(a), of(b)));
}

[[nodiscard]] F32x8 F32x8::mul(F32x8 a, F32x8 b) noexcept {
    return to(_mm256_mul_ps(of(a), of(b)));
}

[[nodiscard]] F32x8 F32x8::div(F32x8 a, F32x8 b) noexcept {
    return to(_mm256_div_ps(of(a), of(b)));
}

[[nodiscard]] F32x8 F32x8::min(F32x8 a, F32x8 b) noexcept {
    return to(_mm256_min_ps(of(a), of(b)));
}

[[nodiscard]] F32x8 F32x8::max(F32x8 a, F32x8 b) noexcept {
    return to(_mm256_max_ps(of(a), of(b)));
}

[[nodiscard]] F32x8 F32x8::floor(F32x8 a) noexcept {
    return to(_mm256_floor_ps(of(a)));
}

[[nodiscard]] F32x8 F32x8::ceil(F32x8 a) noexcept {
    return to(_mm256_ceil_ps(of(a)));
}

[[nodiscard]] F32x8 F32x8::abs(F32x8 a) noexcept {
    return to(_mm256_andnot_ps(_mm256_set1_ps(-0.0f), of(a)));
}

[[nodiscard]] f32 F32x8::dp(F32x8 a, F32x8 b) noexcept {
    return _mm256_cvtss_f32(_mm256_dp_ps(of(a), of(b), 0xff));
}

[[nodiscard]] i32 F32x8::cmpeq(F32x8 a, F32x8 b) noexcept {
    return to(_mm256_movemask_ps(_mm256_cmp_ps(of(a), of(b), 0));
}

#else

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

#endif  // RPP_COMPILER_MSVC

} // namespace rpp::SIMD
