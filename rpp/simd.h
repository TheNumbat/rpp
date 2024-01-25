
#pragma once

#include "base.h"

// The implementation of these functions are compiled with SSE on MSVC x86-64 and gcc/clang vector
// intrinsics on everything else.

namespace rpp::SIMD {

struct F32x4 {
#ifdef RPP_COMPILER_MSVC
    alignas(16) f32 data[4];
#else
    using f32x4 = f32 __attribute__((ext_vector_type(4)));
    f32x4 data;
#endif

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
    [[nodiscard]] static bool cmpeq_all(F32x4 a, F32x4 b) noexcept;
};

} // namespace rpp::SIMD
