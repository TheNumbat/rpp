
#include "test.h"
#include <rpp/math.h>
#include <rpp/simd.h>

#define assert_near(a, b) assert(rpp::Math::abs((a) - (b)) < 0.001)

using namespace rpp::SIMD;

i32 main() {
    Trace("SetZero4") {
        F32x4 f = F32x4::zero();
        assert(f.data[0] == 0.f);
        assert(f.data[1] == 0.f);
        assert(f.data[2] == 0.f);
        assert(f.data[3] == 0.f);
    }

    Trace("SetOne4") {
        F32x4 f = F32x4::one();
        assert(f.data[0] == 1.0);
        assert(f.data[1] == 1.0);
        assert(f.data[2] == 1.0);
        assert(f.data[3] == 1.0);
    }

    Trace("SetXYZW") {
        F32x4 f = F32x4::set(4.5f, 3.2f, 55.f, -1000.f);
        assert(f.data[0] == 4.5f);
        assert(f.data[1] == 3.2f);
        assert(f.data[2] == 55.f);
        assert(f.data[3] == -1000.f);
    }

    Trace("SetABCDEFGH") {
        F32x8 f = F32x8::set(4.5f, 3.2f, 55.f, -1000.f, 45.3f, 55.7f, -200.4f, 0.f);
        assert(f.data[0] == 4.5f);
        assert(f.data[1] == 3.2f);
        assert(f.data[2] == 55.f);
        assert(f.data[3] == -1000.f);
        assert(f.data[4] == 45.3f);
        assert(f.data[5] == 55.7f);
        assert(f.data[6] == -200.4f);
        assert(f.data[7] == 0.f);
    }

    Trace("Operations4") {
        F32x4 f = F32x4::set(99.4f, -12.4f, 44.232f, 408.f);
        F32x4 g = F32x4::set(384.4f, -45.3f, -99.3f, 408.f);

        F32x4 max = F32x4::max(f, g);
        assert_near(max.data[0], 384.4);
        assert_near(max.data[1], -12.4);
        assert_near(max.data[2], 44.232);
        assert_near(max.data[3], 408);

        F32x4 min = F32x4::min(f, g);
        assert_near(min.data[0], 99.4);
        assert_near(min.data[1], -45.3);
        assert_near(min.data[2], -99.3);
        assert_near(min.data[3], 408);

        F32x4 floor = F32x4::floor(f);
        assert_near(floor.data[0], 99);
        assert_near(floor.data[1], -13);
        assert_near(floor.data[2], 44);
        assert_near(floor.data[3], 408);

        F32x4 ceil = F32x4::ceil(f);
        assert_near(ceil.data[0], 100);
        assert_near(ceil.data[1], -12);
        assert_near(ceil.data[2], 45);
        assert_near(ceil.data[3], 408);

        F32x4 abs = F32x4::abs(f);
        assert_near(abs.data[0], 99.4);
        assert_near(abs.data[1], 12.4);
        assert_near(abs.data[2], 44.232);
        assert_near(abs.data[3], 408);

        i32 cmp0 = F32x4::cmpeq(f, g);
        assert(cmp0 != ~0x0);
        assert(!cmp0); // == 0
        i32 cmp1 = F32x4::cmpeq(f, f);
        assert(cmp1 == ~0x0); // all true
        assert(cmp1);         // != 0
        i32 cmp2 = F32x4::cmpeq(g, g);
        assert(cmp2 == ~0x0);
        assert(cmp2);
        i32 cmp3 = F32x4::cmpeq(F32x4::zero(), F32x4::zero());
        assert(cmp3 == ~0x0);
        assert(cmp3);
        i32 cmp4 = F32x4::cmpeq(F32x4::one(), F32x4::zero());
        assert(cmp4 != ~0x0);
        assert(!cmp4);
    }

    Trace("Arithmetic4") {
        F32x4 a = F32x4::set(4.5f, 3.2f, 55.f, -1000.f);
        F32x4 b = F32x4::set(9.5f, 2.f, 200.f, 0.1f);

        F32x4 sum = F32x4::add(a, b);
        assert_near(sum.data[0], 4.5f + 9.5f);
        assert_near(sum.data[1], 3.2f + 2.f);
        assert_near(sum.data[2], 55.f + 200.f);
        assert_near(sum.data[3], -1000.f + 0.1f);

        F32x4 sub = F32x4::sub(a, b);
        assert_near(sub.data[0], 4.5f - 9.5f);
        assert_near(sub.data[1], 3.2f - 2.f);
        assert_near(sub.data[2], 55.f - 200.f);
        assert_near(sub.data[3], -1000.f - 0.1f);

        F32x4 mul = F32x4::mul(a, b);
        assert_near(mul.data[0], 4.5f * 9.5f);
        assert_near(mul.data[1], 3.2f * 2.f);
        assert_near(mul.data[2], 55.f * 200.f);
        assert_near(mul.data[3], -1000.f * 0.1f);

        F32x4 div = F32x4::div(a, b);
        assert_near(div.data[0], 4.5f / 9.5f);
        assert_near(div.data[1], 3.2f / 2.f);
        assert_near(div.data[2], 55.f / 200.f);
        assert_near(div.data[3], -1000.f / 0.1f);

        f32 dot = F32x4::dp(a, b);
        assert_near(dot, 10949.15);
        f32 dot_rev = F32x4::dp(b, a);
        assert_near(dot_rev, 10949.15); // should be commutative
    }

    return 0;
}
