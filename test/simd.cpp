
#include "test.h"
#include <rpp/math.h>
#include <rpp/simd.h>

#define assert_near(a, b) assert(rpp::Math::abs((a) - (b)) < 0.001f)

using namespace rpp::SIMD;

i32 main() {
    Trace("SetZero4") {
        F32x4 f = F32x4::zero();
        assert_near(f.data[0], 0.0f);
        assert_near(f.data[1], 0.0f);
        assert_near(f.data[2], 0.0f);
        assert_near(f.data[3], 0.0f);
    }

    Trace("SetOne4") {
        F32x4 f = F32x4::one();
        assert_near(f.data[0], 1.0f);
        assert_near(f.data[1], 1.0f);
        assert_near(f.data[2], 1.0f);
        assert_near(f.data[3], 1.0f);
    }

    Trace("SetXYZW") {
        F32x4 f = F32x4::set(4.5f, 3.2f, 55.f, -1000.f);
        assert_near(f.data[0], 4.5f);
        assert_near(f.data[1], 3.2f);
        assert_near(f.data[2], 55.0f);
        assert_near(f.data[3], -1000.0f);
    }

    Trace("Operations4") {
        F32x4 f = F32x4::set(99.4f, -12.4f, 44.232f, 408.0f);
        F32x4 g = F32x4::set(384.4f, -45.3f, -99.3f, 408.0f);

        F32x4 max = F32x4::max(f, g);
        assert_near(max.data[0], 384.4f);
        assert_near(max.data[1], -12.4f);
        assert_near(max.data[2], 44.232f);
        assert_near(max.data[3], 408.0f);

        F32x4 min = F32x4::min(f, g);
        assert_near(min.data[0], 99.4f);
        assert_near(min.data[1], -45.3f);
        assert_near(min.data[2], -99.3f);
        assert_near(min.data[3], 408.0f);

        F32x4 floor = F32x4::floor(f);
        assert_near(floor.data[0], 99.0f);
        assert_near(floor.data[1], -13.0f);
        assert_near(floor.data[2], 44.0f);
        assert_near(floor.data[3], 408.0f);

        F32x4 ceil = F32x4::ceil(f);
        assert_near(ceil.data[0], 100.0f);
        assert_near(ceil.data[1], -12.0f);
        assert_near(ceil.data[2], 45.0f);
        assert_near(ceil.data[3], 408.0f);

        F32x4 abs = F32x4::abs(f);
        assert_near(abs.data[0], 99.4f);
        assert_near(abs.data[1], 12.4f);
        assert_near(abs.data[2], 44.232f);
        assert_near(abs.data[3], 408.0f);

        assert(!F32x4::cmpeq_all(f, g));
        assert(F32x4::cmpeq_all(f, f));
        assert(F32x4::cmpeq_all(g, g));
        assert(F32x4::cmpeq_all(F32x4::zero(), F32x4::zero()));
        assert(!F32x4::cmpeq_all(F32x4::one(), F32x4::zero()));
    }

    Trace("Arithmetic4") {
        F32x4 a = F32x4::set(4.5f, 3.2f, 55.0f, -1000.0f);
        F32x4 b = F32x4::set(9.5f, 2.0f, 200.0f, 0.1f);

        F32x4 sum = F32x4::add(a, b);
        assert_near(sum.data[0], 4.5f + 9.5f);
        assert_near(sum.data[1], 3.2f + 2.0f);
        assert_near(sum.data[2], 55.0f + 200.0f);
        assert_near(sum.data[3], -1000.0f + 0.1f);

        F32x4 sub = F32x4::sub(a, b);
        assert_near(sub.data[0], 4.5f - 9.5f);
        assert_near(sub.data[1], 3.2f - 2.0f);
        assert_near(sub.data[2], 55.0f - 200.0f);
        assert_near(sub.data[3], -1000.0f - 0.1f);

        F32x4 mul = F32x4::mul(a, b);
        assert_near(mul.data[0], 4.5f * 9.5f);
        assert_near(mul.data[1], 3.2f * 2.0f);
        assert_near(mul.data[2], 55.0f * 200.0f);
        assert_near(mul.data[3], -1000.0f * 0.1f);

        F32x4 div = F32x4::div(a, b);
        assert_near(div.data[0], 4.5f / 9.5f);
        assert_near(div.data[1], 3.2f / 2.0f);
        assert_near(div.data[2], 55.0f / 200.0f);
        assert_near(div.data[3], -1000.0f / 0.1f);

        f32 dot = F32x4::dp(a, b);
        assert_near(dot, 10949.15f);
        f32 dot_rev = F32x4::dp(b, a);
        assert_near(dot_rev, 10949.15f); // should be commutative
    }

    return 0;
}
