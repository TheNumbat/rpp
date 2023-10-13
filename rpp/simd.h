
#pragma once

#include "base.h"

namespace rpp::SIMD {

struct F32x4 {
    alignas(16) f32 data[4];

    static constexpr F32x4 set1(f32 v) {
        return {v, v, v, v};
    }
    static constexpr F32x4 set(f32 x, f32 y, f32 z, f32 w) {
        return {x, y, z, w};
    }
    static constexpr F32x4 zero() {
        return {0.0f, 0.0f, 0.0f, 0.0f};
    }
    static constexpr F32x4 one() {
        return {1.0f, 1.0f, 1.0f, 1.0f};
    }

    static F32x4 add(F32x4 a, F32x4 b);
    static F32x4 sub(F32x4 a, F32x4 b);
    static F32x4 mul(F32x4 a, F32x4 b);
    static F32x4 div(F32x4 a, F32x4 b);
    static F32x4 min(F32x4 a, F32x4 b);
    static F32x4 max(F32x4 a, F32x4 b);
    static F32x4 floor(F32x4 a);
    static F32x4 ceil(F32x4 a);
    static F32x4 abs(F32x4 a);
    static f32 dp(F32x4 a, F32x4 b);
    static F32x4 cmpeq(F32x4 a, F32x4 b);
    static i32 movemask(F32x4 a);
};

} // namespace rpp::SIMD
