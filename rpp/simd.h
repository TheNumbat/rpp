
#pragma once

#include "base.h"

namespace rpp::SIMD {

struct F32x4 {
    alignas(16) f32 data[4];

    [[nodiscard]] static F32x4 set1(f32 v) noexcept;
    [[nodiscard]] static F32x4 set(f32 x, f32 y, f32 z, f32 w) noexcept;
    [[nodiscard]] static F32x4 zero() noexcept;
    [[nodiscard]] static F32x4 one() noexcept;
    [[nodiscard]] static F32x4 add(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 sub(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 mul(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 div(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 min(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 max(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 floor(F32x4 a) noexcept;
    [[nodiscard]] static F32x4 ceil(F32x4 a) noexcept;
    [[nodiscard]] static F32x4 abs(F32x4 a) noexcept;
    [[nodiscard]] static f32 dp(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 cmpeq(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static i32 movemask(F32x4 a) noexcept;
};

struct F32x8 {
    alignas(32) f32 data[8];

    [[nodiscard]] static F32x8 set1(f32 v) noexcept;
    [[nodiscard]] static F32x8 set(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h) noexcept;
    [[nodiscard]] static F32x8 zero() noexcept;
    [[nodiscard]] static F32x8 one() noexcept;
    [[nodiscard]] static F32x8 add(F32x8 a, F32x8 b) noexcept;
    [[nodiscard]] static F32x8 sub(F32x8 a, F32x8 b) noexcept;
    [[nodiscard]] static F32x8 mul(F32x8 a, F32x8 b) noexcept;
    [[nodiscard]] static F32x8 div(F32x8 a, F32x8 b) noexcept;
    [[nodiscard]] static F32x8 min(F32x8 a, F32x8 b) noexcept;
    [[nodiscard]] static F32x8 max(F32x8 a, F32x8 b) noexcept;
    [[nodiscard]] static F32x8 floor(F32x8 a) noexcept;
    [[nodiscard]] static F32x8 ceil(F32x8 a) noexcept;
    [[nodiscard]] static F32x8 abs(F32x8 a) noexcept;
    [[nodiscard]] static f32 dp(F32x8 a, F32x8 b) noexcept;
    [[nodiscard]] static F32x8 cmpeq(F32x8 a, F32x8 b) noexcept;
    [[nodiscard]] static i32 movemask(F32x8 a) noexcept;
};

} // namespace rpp::SIMD
