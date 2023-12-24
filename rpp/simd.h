
#pragma once

#include "base.h"

namespace rpp::SIMD {

struct F32x4 {
    alignas(16) f32 data[4];

    static F32x4 set1(f32 v);
    static F32x4 set(f32 x, f32 y, f32 z, f32 w);
    static F32x4 zero();
    static F32x4 one();
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

struct F32x8 {
    alignas(32) f32 data[4];

    static F32x8 set1(f32 v);
    static F32x8 set(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h);
    static F32x8 zero();
    static F32x8 one();
    static F32x8 add(F32x8 a, F32x8 b);
    static F32x8 sub(F32x8 a, F32x8 b);
    static F32x8 mul(F32x8 a, F32x8 b);
    static F32x8 div(F32x8 a, F32x8 b);
    static F32x8 min(F32x8 a, F32x8 b);
    static F32x8 max(F32x8 a, F32x8 b);
    static F32x8 floor(F32x8 a);
    static F32x8 ceil(F32x8 a);
    static F32x8 abs(F32x8 a);
    static f32 dp(F32x8 a, F32x8 b);
    static F32x8 cmpeq(F32x8 a, F32x8 b);
    static i32 movemask(F32x8 a);
};

} // namespace rpp::SIMD
