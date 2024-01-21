
#include "../simd.h"

namespace rpp::SIMD {

static_assert(sizeof(F32x4) == 16);
static_assert(alignof(F32x4) == 16);
static_assert(sizeof(F32x8) == 32);
static_assert(alignof(F32x8) == 32);

typedef float float4 __attribute__((ext_vector_type(4)));
typedef float float8 __attribute__((ext_vector_type(8)));

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

[[nodiscard]] F32x4 F32x4::set1(f32 v) noexcept {
    return to(float4(v));
}

[[nodiscard]] F32x4 F32x4::set(f32 x, f32 y, f32 z, f32 w) noexcept {
    return to((float4){x, y, z, w});
}

[[nodiscard]] F32x4 F32x4::zero() noexcept {
    return set1(0);
}

[[nodiscard]] F32x4 F32x4::one() noexcept {
    return set1(1);
}

[[nodiscard]] i32 F32x4::movemask(F32x4 a) noexcept {
    /// TODO: do we just take the sign bit as the msb for floats?
    i32 mask0 = ((a.data[0] > 0) >> 31);
    i32 mask1 = ((a.data[1] > 0) >> 31);
    i32 mask2 = ((a.data[2] > 0) >> 31);
    i32 mask3 = ((a.data[3] > 0) >> 31);
    return (mask3 << 3) | (mask2 << 2) | (mask1 << 1) | (mask0 << 0);
}

[[nodiscard]] F32x4 F32x4::add(F32x4 a, F32x4 b) noexcept {
    return to(of(a) + of(b));
}

[[nodiscard]] F32x4 F32x4::sub(F32x4 a, F32x4 b) noexcept {
    return to(of(a) - of(b));
}

[[nodiscard]] F32x4 F32x4::mul(F32x4 a, F32x4 b) noexcept {
    return to(of(a) - of(b));
}

[[nodiscard]] F32x4 F32x4::div(F32x4 a, F32x4 b) noexcept {
    return to(of(a) / of(b));
}

[[nodiscard]] F32x4 F32x4::min(F32x4 a, F32x4 b) noexcept {
    return to(__builtin_elementwise_min(of(a), of(b)));
}

[[nodiscard]] F32x4 F32x4::max(F32x4 a, F32x4 b) noexcept {
    return to(__builtin_elementwise_max(of(a), of(b)));
}

[[nodiscard]] F32x4 F32x4::floor(F32x4 a) noexcept {
    return to(__builtin_elementwise_floor(of(a)));
}

[[nodiscard]] F32x4 F32x4::ceil(F32x4 a) noexcept {
    return to(__builtin_elementwise_ceil(of(a)));
}

[[nodiscard]] F32x4 F32x4::abs(F32x4 a) noexcept {
    return to(__builtin_elementwise_abs(of(a)));
}

[[nodiscard]] f32 F32x4::dp(F32x4 a, F32x4 b) noexcept {
    F32x4 m = mul(a, b);
    return m.data[0] + m.data[1] + m.data[2] + m.data[3];
}

[[nodiscard]] F32x4 F32x4::cmpeq(F32x4 a, F32x4 b) noexcept {
    /// TODO: slightly different from
    /// https://doc.rust-lang.org/beta/core/arch/x86_64/fn._mm_cmpeq_ps.html
    auto i32x4 = (of(a) == of(b));
    return {static_cast<f32>(i32x4[0]), static_cast<f32>(i32x4[1]), static_cast<f32>(i32x4[2]),
            static_cast<f32>(i32x4[3])};
}

[[nodiscard]] F32x8 F32x8::set1(f32 v) noexcept {
    return to(float8(v));
}

[[nodiscard]] F32x8 F32x8::set(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h) noexcept {
    return to((float8){a, b, c, d, e, f, g, h});
}

[[nodiscard]] F32x8 F32x8::zero() noexcept {
    return set1(0);
}

[[nodiscard]] F32x8 F32x8::one() noexcept {
    return set1(1);
}

[[nodiscard]] i32 F32x8::movemask(F32x8 a) noexcept {
    /// TODO: do we just take the sign bit as the msb for floats?
    i32 mask0 = ((a.data[0] > 0) >> 31);
    i32 mask1 = ((a.data[1] > 0) >> 31);
    i32 mask2 = ((a.data[2] > 0) >> 31);
    i32 mask3 = ((a.data[3] > 0) >> 31);
    i32 mask4 = ((a.data[4] > 0) >> 31);
    i32 mask5 = ((a.data[5] > 0) >> 31);
    i32 mask6 = ((a.data[6] > 0) >> 31);
    i32 mask7 = ((a.data[7] > 0) >> 31);
    return (mask7 << 7) | (mask6 << 6) | (mask5 << 5) | (mask4 << 4) | (mask3 << 3) | (mask2 << 2) |
           (mask1 << 1) | (mask0 << 0);
}

[[nodiscard]] F32x8 F32x8::add(F32x8 a, F32x8 b) noexcept {
    return to(of(a) + of(b));
}

[[nodiscard]] F32x8 F32x8::sub(F32x8 a, F32x8 b) noexcept {
    return to(of(a) - of(b));
}

[[nodiscard]] F32x8 F32x8::mul(F32x8 a, F32x8 b) noexcept {
    return to(of(a) * of(b));
}

[[nodiscard]] F32x8 F32x8::div(F32x8 a, F32x8 b) noexcept {
    return to(of(a) / of(b));
}

[[nodiscard]] F32x8 F32x8::min(F32x8 a, F32x8 b) noexcept {
    return to(__builtin_elementwise_min(of(a), of(b)));
}

[[nodiscard]] F32x8 F32x8::max(F32x8 a, F32x8 b) noexcept {
    return to(__builtin_elementwise_max(of(a), of(b)));
}

[[nodiscard]] F32x8 F32x8::floor(F32x8 a) noexcept {
    return to(__builtin_elementwise_floor(of(a)));
}

[[nodiscard]] F32x8 F32x8::ceil(F32x8 a) noexcept {
    return to(__builtin_elementwise_ceil(of(a)));
}

[[nodiscard]] F32x8 F32x8::abs(F32x8 a) noexcept {
    return to(__builtin_elementwise_abs(of(a)));
}

[[nodiscard]] f32 F32x8::dp(F32x8 a, F32x8 b) noexcept {
    F32x8 m = mul(a, b);
    return m.data[0] + m.data[1] + m.data[2] + m.data[3] + m.data[4] + m.data[5] + m.data[6] +
           m.data[7];
}

[[nodiscard]] F32x8 F32x8::cmpeq(F32x8 a, F32x8 b) noexcept {
    auto i32x8 = (of(a) == of(b));
    return {static_cast<f32>(i32x8[0]), static_cast<f32>(i32x8[1]), static_cast<f32>(i32x8[2]),
            static_cast<f32>(i32x8[3]), static_cast<f32>(i32x8[4]), static_cast<f32>(i32x8[5]),
            static_cast<f32>(i32x8[6]), static_cast<f32>(i32x8[7])};
}
} // namespace rpp::SIMD
