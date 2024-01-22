
#include "test.h"
#include <rpp/vmath.h>

using namespace rpp::Math;

#define assert_near(a, b) assert(rpp::Math::abs((a) - (b)) < 0.001)

i32 main() {
    Trace("MatrixInverse") {
        Mat4 m = Mat4{3.4f, 2.2f,  5.2f, 8.4f, 0.4f, 22.f, 9.3f, -10.2f,
                      0.4f, 10.3f, 9.8f, 3.4f, 0.4f, 5.5f, 3.5f, 6.5f};
        Mat4 inv = Mat4::inverse(m);
        // expected:
        Mat4 e =
            Mat4{0.32380925f,  0.10641495f,  -0.22503008f, -0.13376353f, -0.0129428f, 0.05508591f,
                 -0.10114984f, 0.15607773f,  0.00430437f,  -0.0538581f,  0.22508721f, -0.20781629f,
                 -0.01129286f, -0.02415925f, -0.02176447f, 0.14191306f};
        assert_near(inv[0][0], e[0][0]);
        assert_near(inv[0][1], e[0][1]);
        assert_near(inv[0][2], e[0][2]);
        assert_near(inv[0][3], e[0][3]);
        assert_near(inv[1][0], e[1][0]);
        assert_near(inv[1][1], e[1][1]);
        assert_near(inv[1][2], e[1][2]);
        assert_near(inv[1][3], e[1][3]);
        assert_near(inv[2][0], e[2][0]);
        assert_near(inv[2][1], e[2][1]);
        assert_near(inv[2][2], e[2][2]);
        assert_near(inv[2][3], e[2][3]);
        assert_near(inv[3][0], e[3][0]);
        assert_near(inv[3][1], e[3][1]);
        assert_near(inv[3][2], e[3][2]);
        assert_near(inv[3][3], e[3][3]);
    }

    Trace("MatrixIdentityInverse") {
        Mat4 m = Mat4::I;
        Mat4 inv = Mat4::inverse(m);
        assert(m == inv);
    }
    return 0;
}
