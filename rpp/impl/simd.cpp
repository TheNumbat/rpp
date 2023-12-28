
#include "../simd.h"

#include <immintrin.h>

namespace rpp::SIMD {

static_assert(sizeof(F32x4) == 16);
static_assert(alignof(F32x4) == 16);
static_assert(sizeof(F32x8) == 32);
static_assert(alignof(F32x8) == 32);

static __m128 of(F32x4 a) {
    return *reinterpret_cast<__m128*>(a.data);
}

static F32x4 to(__m128 a) {
    return *reinterpret_cast<F32x4*>(&a);
}

static __m256 of(F32x8 a) {
    return *reinterpret_cast<__m256*>(a.data);
}

static F32x8 to(__m256 a) {
    return *reinterpret_cast<F32x8*>(&a);
}

F32x4 F32x4::set1(f32 v) {
    return to(_mm_set1_ps(v));
}

F32x4 F32x4::set(f32 x, f32 y, f32 z, f32 w) {
    return to(_mm_set_ps(x, y, z, w));
}

F32x4 F32x4::zero() {
    return to(_mm_setzero_ps());
}

F32x4 F32x4::one() {
    return to(_mm_setzero_ps());
}

i32 F32x4::movemask(F32x4 a) {
    return _mm_movemask_ps(of(a));
}

F32x4 F32x4::add(F32x4 a, F32x4 b) {
    return to(_mm_add_ps(of(a), of(b)));
}

F32x4 F32x4::sub(F32x4 a, F32x4 b) {
    return to(_mm_sub_ps(of(a), of(b)));
}

F32x4 F32x4::mul(F32x4 a, F32x4 b) {
    return to(_mm_mul_ps(of(a), of(b)));
}

F32x4 F32x4::div(F32x4 a, F32x4 b) {
    return to(_mm_div_ps(of(a), of(b)));
}

F32x4 F32x4::min(F32x4 a, F32x4 b) {
    return to(_mm_min_ps(of(a), of(b)));
}

F32x4 F32x4::max(F32x4 a, F32x4 b) {
    return to(_mm_max_ps(of(a), of(b)));
}

F32x4 F32x4::floor(F32x4 a) {
    return to(_mm_floor_ps(of(a)));
}

F32x4 F32x4::ceil(F32x4 a) {
    return to(_mm_ceil_ps(of(a)));
}

F32x4 F32x4::abs(F32x4 a) {
    return to(_mm_andnot_ps(_mm_set1_ps(-0.0f), of(a)));
}

f32 F32x4::dp(F32x4 a, F32x4 b) {
    return _mm_cvtss_f32(_mm_dp_ps(of(a), of(b), 0xff));
}

F32x4 F32x4::cmpeq(F32x4 a, F32x4 b) {
    return to(_mm_cmpeq_ps(of(a), of(b)));
}

F32x8 F32x8::set1(f32 v) {
    return to(_mm256_set1_ps(v));
}

F32x8 F32x8::set(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h) {
    return to(_mm256_set_ps(a, b, c, d, e, f, g, h));
}

F32x8 F32x8::zero() {
    return to(_mm256_setzero_ps());
}

F32x8 F32x8::one() {
    return to(_mm256_setzero_ps());
}

i32 F32x8::movemask(F32x8 a) {
    return _mm256_movemask_ps(of(a));
}

F32x8 F32x8::add(F32x8 a, F32x8 b) {
    return to(_mm256_add_ps(of(a), of(b)));
}

F32x8 F32x8::sub(F32x8 a, F32x8 b) {
    return to(_mm256_sub_ps(of(a), of(b)));
}

F32x8 F32x8::mul(F32x8 a, F32x8 b) {
    return to(_mm256_mul_ps(of(a), of(b)));
}

F32x8 F32x8::div(F32x8 a, F32x8 b) {
    return to(_mm256_div_ps(of(a), of(b)));
}

F32x8 F32x8::min(F32x8 a, F32x8 b) {
    return to(_mm256_min_ps(of(a), of(b)));
}

F32x8 F32x8::max(F32x8 a, F32x8 b) {
    return to(_mm256_max_ps(of(a), of(b)));
}

F32x8 F32x8::floor(F32x8 a) {
    return to(_mm256_floor_ps(of(a)));
}

F32x8 F32x8::ceil(F32x8 a) {
    return to(_mm256_ceil_ps(of(a)));
}

F32x8 F32x8::abs(F32x8 a) {
    return to(_mm256_andnot_ps(_mm256_set1_ps(-0.0f), of(a)));
}

f32 F32x8::dp(F32x8 a, F32x8 b) {
    return _mm256_cvtss_f32(_mm256_dp_ps(of(a), of(b), 0xff));
}

F32x8 F32x8::cmpeq(F32x8 a, F32x8 b) {
    return to(_mm256_cmp_ps(of(a), of(b), 0));
}

} // namespace rpp::SIMD
