
#include "../base.h"

#ifdef RPP_COMPILER_MSVC
#include <intrin.h>
#endif

#include <math.h>

namespace rpp::Math {

[[nodiscard]] f32 cos(f32 v) noexcept {
    return ::cosf(v);
}
[[nodiscard]] f64 cos(f64 v) noexcept {
    return ::cos(v);
}
[[nodiscard]] f32 sin(f32 v) noexcept {
    return ::sinf(v);
}
[[nodiscard]] f64 sin(f64 v) noexcept {
    return ::sin(v);
}
[[nodiscard]] f32 tan(f32 v) noexcept {
    return ::tanf(v);
}
[[nodiscard]] f64 tan(f64 v) noexcept {
    return ::tan(v);
}
[[nodiscard]] f32 acos(f32 v) noexcept {
    return ::acosf(v);
}
[[nodiscard]] f64 acos(f64 v) noexcept {
    return ::acos(v);
}
[[nodiscard]] f32 asin(f32 v) noexcept {
    return ::asinf(v);
}
[[nodiscard]] f64 asin(f64 v) noexcept {
    return ::asin(v);
}
[[nodiscard]] f32 atan(f32 v) noexcept {
    return ::atanf(v);
}
[[nodiscard]] f64 atan(f64 v) noexcept {
    return ::atan(v);
}
[[nodiscard]] f32 atan2(f32 y, f32 x) noexcept {
    return ::atan2f(y, x);
}
[[nodiscard]] f64 atan2(f64 y, f64 x) noexcept {
    return ::atan2(y, x);
}
[[nodiscard]] f32 hypot(f32 x, f32 y) noexcept {
    return ::hypotf(x, y);
}
[[nodiscard]] f64 hypot(f64 x, f64 y) noexcept {
    return ::hypot(x, y);
}
[[nodiscard]] f32 pow(f32 x, f32 y) noexcept {
    return ::powf(x, y);
}
[[nodiscard]] f64 pow(f64 x, f64 y) noexcept {
    return ::pow(x, y);
}
[[nodiscard]] f32 exp(f32 x) noexcept {
    return ::expf(x);
}
[[nodiscard]] f64 exp(f64 x) noexcept {
    return ::exp(x);
}
[[nodiscard]] f32 floor(f32 v) noexcept {
    return ::floorf(v);
}
[[nodiscard]] f64 floor(f64 v) noexcept {
    return ::floor(v);
}
[[nodiscard]] f32 ceil(f32 v) noexcept {
    return ::ceilf(v);
}
[[nodiscard]] f64 ceil(f64 v) noexcept {
    return ::ceil(v);
}
[[nodiscard]] f32 round(f32 v) noexcept {
    return ::roundf(v);
}
[[nodiscard]] f64 round(f64 v) noexcept {
    return ::round(v);
}
[[nodiscard]] f32 abs(f32 v) noexcept {
    return ::fabsf(v);
}
[[nodiscard]] f64 abs(f64 v) noexcept {
    return ::fabs(v);
}
[[nodiscard]] f32 sqrt(f32 v) noexcept {
    return ::sqrtf(v);
}
[[nodiscard]] f64 sqrt(f64 v) noexcept {
    return ::sqrt(v);
}
[[nodiscard]] f32 sign(f32 v) noexcept {
    return v == 0.0f ? 0.0f : v < 0.0f ? -1.0f : 1.0f;
}
[[nodiscard]] f64 sign(f64 v) noexcept {
    return v == 0.0 ? 0.0 : v < 0.0 ? -1.0 : 1.0;
}
[[nodiscard]] i32 abs(i32 v) noexcept {
    return ::abs(v);
}
[[nodiscard]] i64 abs(i64 v) noexcept {
    return ::llabs(v);
}

[[nodiscard]] u32 popcount(u32 val) noexcept {
#ifdef RPP_COMPILER_MSVC
    return __popcnt(val);
#else
    return __builtin_popcount(val);
#endif
}

[[nodiscard]] u64 popcount(u64 val) noexcept {
#ifdef RPP_COMPILER_MSVC
    return __popcnt64(val);
#else
    return __builtin_popcountll(val);
#endif
}

[[nodiscard]] u32 ctlz(u32 val) noexcept {
#ifdef RPP_COMPILER_MSVC
    return __lzcnt(val);
#else
    if(val == 0) return 32;
    return __builtin_clz(val);
#endif
}

[[nodiscard]] u64 ctlz(u64 val) noexcept {
#ifdef RPP_COMPILER_MSVC
    return __lzcnt64(val);
#else
    if(val == 0) return 64;
    return __builtin_clzll(val);
#endif
}

[[nodiscard]] u32 log2(u32 val) noexcept {
    return 31u - ctlz(val);
}
[[nodiscard]] u64 log2(u64 val) noexcept {
    return 63ull - ctlz(val);
}

[[nodiscard]] u32 prev_pow2(u32 val) noexcept {
    if(val == 0) return 0;
    return 1u << log2(val);
}
[[nodiscard]] u64 prev_pow2(u64 val) noexcept {
    if(val == 0) return 0;
    return 1ull << log2(val);
}

[[nodiscard]] u32 next_pow2(u32 x) noexcept {
    u32 prev = prev_pow2(x);
    return x == prev ? x : prev << 1u;
}
[[nodiscard]] u64 next_pow2(u64 x) noexcept {
    u64 prev = prev_pow2(x);
    return x == prev ? x : prev << 1ull;
}

} // namespace rpp::Math
