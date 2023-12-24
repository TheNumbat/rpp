
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

// clang-format off

#define RPP_DBL_EPSILON      2.2204460492503131e-016 // smallest such that 1.0+DBL_EPSILON != 1.0
#define RPP_DBL_MAX          1.7976931348623158e+308 // max value
#define RPP_DBL_MIN          2.2250738585072014e-308 // min positive norm
#define RPP_DBL_TRUE_MIN     4.9406564584124654e-324 // min positive denorm
#define RPP_FLT_EPSILON      1.192092896e-07F        // smallest such that 1.0+FLT_EPSILON != 1.0
#define RPP_FLT_MAX          3.402823466e+38F        // max value
#define RPP_FLT_MIN          1.175494351e-38F        // min positive norm
#define RPP_FLT_TRUE_MIN     1.401298464e-45F        // min positive denorm
#define RPP_INT8_MIN         (-127 - 1)
#define RPP_INT16_MIN        (-32767 - 1)
#define RPP_INT32_MIN        (-2147483647 - 1)
#define RPP_INT64_MIN        (-9223372036854775807 - 1)
#define RPP_INT8_MAX         127
#define RPP_INT16_MAX        32767
#define RPP_INT32_MAX        2147483647
#define RPP_INT64_MAX        9223372036854775807
#define RPP_UINT8_MAX        0xff
#define RPP_UINT16_MAX       0xffff
#define RPP_UINT32_MAX       0xffffffff
#define RPP_UINT64_MAX       0xffffffffffffffff

// clang-format on

namespace rpp {

template<typename T>
struct Limits;

template<>
struct Limits<u8> {
    static constexpr u8 max() {
        return RPP_UINT8_MAX;
    }
    static constexpr u8 min() {
        return 0;
    }
};

template<>
struct Limits<u16> {
    static constexpr u16 max() {
        return RPP_UINT16_MAX;
    }
    static constexpr u16 min() {
        return 0;
    }
};

template<>
struct Limits<u32> {
    static constexpr u32 max() {
        return RPP_UINT32_MAX;
    }
    static constexpr u32 min() {
        return 0;
    }
};

template<>
struct Limits<u64> {
    static constexpr u64 max() {
        return RPP_UINT64_MAX;
    }
    static constexpr u64 min() {
        return 0;
    }
};

template<>
struct Limits<i8> {
    static constexpr i8 max() {
        return RPP_INT8_MAX;
    }
    static constexpr i8 min() {
        return RPP_INT8_MIN;
    }
};

template<>
struct Limits<i16> {
    static constexpr i16 max() {
        return RPP_INT16_MAX;
    }
    static constexpr i16 min() {
        return RPP_INT16_MIN;
    }
};

template<>
struct Limits<i32> {
    static constexpr i32 max() {
        return RPP_INT32_MAX;
    }
    static constexpr i32 min() {
        return RPP_INT32_MIN;
    }
};

template<>
struct Limits<i64> {
    static constexpr i64 max() {
        return RPP_INT64_MAX;
    }
    static constexpr i64 min() {
        return RPP_INT64_MIN;
    }
};

template<>
struct Limits<f32> {
    static constexpr f32 max() {
        return RPP_FLT_MAX;
    }
    static constexpr f32 min() {
        return -RPP_FLT_MAX;
    }
    static constexpr f32 smallest_norm() {
        return RPP_FLT_MIN;
    }
    static constexpr f32 smallest_denorm() {
        return RPP_FLT_TRUE_MIN;
    }
    static constexpr f32 epsilon() {
        return RPP_FLT_EPSILON;
    }
};

template<>
struct Limits<f64> {
    static constexpr f64 max() {
        return RPP_DBL_MAX;
    }
    static constexpr f64 min() {
        return -RPP_DBL_MAX;
    }
    static constexpr f64 smallest_norm() {
        return RPP_DBL_MIN;
    }
    static constexpr f64 smallest_denorm() {
        return RPP_DBL_TRUE_MIN;
    }
    static constexpr f64 epsilon() {
        return RPP_DBL_EPSILON;
    }
};

} // namespace rpp