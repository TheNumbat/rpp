
#include "../simd.h"

#include <smmintrin.h>

namespace rpp::SIMD {

static_assert(sizeof(F32x4) == 16);
static_assert(alignof(F32x4) == 16);

static __m128 of(F32x4 a) {
    return *reinterpret_cast<__m128*>(a.data);
}

static F32x4 to(__m128 a) {
    return *reinterpret_cast<F32x4*>(&a);
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

} // namespace rpp::SIMD
