
#include "../simd.h"

namespace rpp::SIMD {

static_assert(sizeof(F32x4) == 16);
static_assert(alignof(F32x4) == 16);
static_assert(sizeof(F32x8) == 32);
static_assert(alignof(F32x8) == 32);

typedef f32 float4 __attribute__((ext_vector_type(4)));
typedef f32 float8 __attribute__((ext_vector_type(8)));

[[nodiscard]] static float4 of(F32x4 a) noexcept {
    return {a.data[0], a.data[1], a.data[2], a.data[3]};
}

[[nodiscard]] static F32x4 to(float4 a) noexcept {
    return {a[0], a[1], a[2], a[3]};
}

[[nodiscard]] static float8 of(F32x8 a) noexcept {
    return {a.data[0], a.data[1], a.data[2], a.data[3], a.data[4], a.data[5], a.data[6], a.data[7]};
}

[[nodiscard]] static F32x8 to(float8 a) noexcept {
    return {a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]};
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::set1(f32 v) noexcept {
    return to(floatT(v));
}

template<>
[[nodiscard]] F32x4 F32x4::set(f32 x, f32 y, f32 z, f32 w) noexcept {
    return to((float4){x, y, z, w});
}

template<>
[[nodiscard]] F32x8 F32x8::set(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h) noexcept {
    return to((float8){a, b, c, d, e, f, g, h});
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::zero() noexcept {
    return to(floatT(0));
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::one() noexcept {
    return to(floatT(1));
}

template<i32 T>
[[nodiscard]] i32 F32x<T>::movemask(F32x<T> a) noexcept {
    // Set each bit of mask dst based on the most significant bit of the corresponding packed
    // single-precision (32-bit) floating-point element in a.
    // https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_mm_movemask_ps
    i32 ret = 0;
    for(i32 i = 0; i < T; i++) {
        ret |= (a.data[i] > 0) << i;
    }
    return ret;
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::add(F32x<T> a, F32x<T> b) noexcept {
    return to(of(a) + of(b));
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::sub(F32x<T> a, F32x<T> b) noexcept {
    return to(of(a) - of(b));
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::mul(F32x<T> a, F32x<T> b) noexcept {
    return to(of(a) - of(b));
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::div(F32x<T> a, F32x<T> b) noexcept {
    return to(of(a) / of(b));
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::min(F32x<T> a, F32x<T> b) noexcept {
    return to(__builtin_elementwise_min(of(a), of(b)));
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::max(F32x<T> a, F32x<T> b) noexcept {
    return to(__builtin_elementwise_max(of(a), of(b)));
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::floor(F32x<T> a) noexcept {
    return to(__builtin_elementwise_floor(of(a)));
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::ceil(F32x<T> a) noexcept {
    return to(__builtin_elementwise_ceil(of(a)));
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::abs(F32x<T> a) noexcept {
    return to(__builtin_elementwise_abs(of(a)));
}

template<i32 T>
[[nodiscard]] f32 F32x<T>::dp(F32x<T> a, F32x<T> b) noexcept {
    F32x4 m = mul(a, b);
    // No built-in to reduce float vectors, lets just do it manually
    f32 ret = 0.f;
    for(i32 i = 0; i < T; i++) {
        ret += m.data[i];
    }
    return ret;
}

template<i32 T>
[[nodiscard]] F32x<T> F32x<T>::cmpeq(F32x<T> a, F32x<T> b) noexcept {
    /// NOTE: Implemented following
    /// https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_mm_cmpeq_ps
    const auto cmp_res = (of(a) == of(b));
    F32x<T> ret;
    for(i32 i = 0; i < T; i++) {
        i32 result = cmp_res[i] ? 0xFFFFFFFF : 0;
        // directly assign the bits of the float
        memcpy(&ret.data[0], &result, sizeof(result));
    }
    return ret;
}
} // namespace rpp::SIMD
