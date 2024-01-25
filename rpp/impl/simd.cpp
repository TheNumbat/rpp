
#include "../simd.h"

#ifdef RPP_COMPILER_MSVC
#include <immintrin.h>
#endif // RPP_COMPILER_MSVC

namespace rpp::SIMD {

static_assert(sizeof(F32x4) == 16);
static_assert(alignof(F32x4) == 16);

#ifdef RPP_COMPILER_MSVC

[[nodiscard]] static __m128 of(F32x4 a) noexcept {
    return *reinterpret_cast<__m128*>(a.data);
}

[[nodiscard]] static F32x4 to(__m128 a) noexcept {
    return *reinterpret_cast<F32x4*>(&a);
}

[[nodiscard]] F32x4 F32x4::set1(f32 v) noexcept {
    return to(_mm_set1_ps(v));
}

[[nodiscard]] F32x4 F32x4::set(f32 x, f32 y, f32 z, f32 w) noexcept {
    // Reversed to preserve big-endianness
    return to(_mm_set_ps(w, z, y, x));
}

[[nodiscard]] F32x4 F32x4::zero() noexcept {
    return to(_mm_setzero_ps());
}

[[nodiscard]] F32x4 F32x4::one() noexcept {
    return set1(1.0f);
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

[[nodiscard]] bool F32x4::cmpeq_all(F32x4 a, F32x4 b) noexcept {
    return _mm_movemask_ps(_mm_cmpeq_ps(of(a), of(b))) == 0xf;
}

#else

[[nodiscard]] F32x4 F32x4::set(f32 x, f32 y, f32 z, f32 w) noexcept {
    return {f32x4{x, y, z, w}};
}

[[nodiscard]] F32x4 F32x4::set1(f32 v) noexcept {
    return {f32x4{v, v, v, v}};
}

[[nodiscard]] F32x4 F32x4::zero() noexcept {
    return set1(0.0f);
}

[[nodiscard]] F32x4 F32x4::one() noexcept {
    return set1(1.0f);
}

[[nodiscard]] F32x4 F32x4::add(F32x4 a, F32x4 b) noexcept {
    return {a.data + b.data};
}

[[nodiscard]] F32x4 F32x4::sub(F32x4 a, F32x4 b) noexcept {
    return {a.data - b.data};
}

[[nodiscard]] F32x4 F32x4::mul(F32x4 a, F32x4 b) noexcept {
    return {a.data * b.data};
}

[[nodiscard]] F32x4 F32x4::div(F32x4 a, F32x4 b) noexcept {
    return {a.data / b.data};
}

[[nodiscard]] F32x4 F32x4::min(F32x4 a, F32x4 b) noexcept {
    return {__builtin_elementwise_min(a.data, b.data)};
}

[[nodiscard]] F32x4 F32x4::max(F32x4 a, F32x4 b) noexcept {
    return {__builtin_elementwise_max(a.data, b.data)};
}

[[nodiscard]] F32x4 F32x4::floor(F32x4 a) noexcept {
    return {__builtin_elementwise_floor(a.data)};
}

[[nodiscard]] F32x4 F32x4::ceil(F32x4 a) noexcept {
    return {__builtin_elementwise_ceil(a.data)};
}

[[nodiscard]] F32x4 F32x4::abs(F32x4 a) noexcept {
    return {__builtin_elementwise_abs(a.data)};
}

[[nodiscard]] f32 F32x4::dp(F32x4 a, F32x4 b) noexcept {
    F32x4 m = mul(a, b);
    // No built-in to reduce float vectors
    f32 ret = 0.f;
    for(i32 i = 0; i < 4; i++) {
        ret += m.data[i];
    }
    return ret;
}

[[nodiscard]] bool F32x4::cmpeq_all(F32x4 a, F32x4 b) noexcept {
    // Element-wise comparison produces -1 (all 1's) when equal, 0 otherwise
    // Reducing via & to get equality of all members
    return __builtin_reduce_and(a.data == b.data) == -1;
}

#endif // RPP_COMPILER_MSVC

} // namespace rpp::SIMD
