
#include "../vmath.h"

#include <smmintrin.h>

namespace rpp::Math {

namespace detail {

using SIMD::of;
using SIMD::to;

#define MakeShuffleMask(x, y, z, w) (x | (y << 2) | (z << 4) | (w << 6))
#define VecSwizzleMask(vec, mask) _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w) VecSwizzleMask(vec, MakeShuffleMask(x, y, z, w))
#define VecSwizzle1(vec, x) VecSwizzleMask(vec, MakeShuffleMask(x, x, x, x))
#define VecSwizzle_0022(vec) _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec) _mm_movehdup_ps(vec)
#define VecShuffle(vec1, vec2, x, y, z, w) _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x, y, z, w))
#define VecShuffle_0101(vec1, vec2) _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2) _mm_movehl_ps(vec2, vec1)

static __m128 Mat2Mul(__m128 vec1, __m128 vec2) noexcept {
    return _mm_add_ps(_mm_mul_ps(vec1, VecSwizzle(vec2, 0, 3, 0, 3)),
                      _mm_mul_ps(VecSwizzle(vec1, 1, 0, 3, 2), VecSwizzle(vec2, 2, 1, 2, 1)));
}

static __m128 Mat2AdjMul(__m128 vec1, __m128 vec2) noexcept {
    return _mm_sub_ps(_mm_mul_ps(VecSwizzle(vec1, 3, 3, 0, 0), vec2),
                      _mm_mul_ps(VecSwizzle(vec1, 1, 1, 2, 2), VecSwizzle(vec2, 2, 3, 0, 1)));
}

static __m128 Mat2MulAdj(__m128 vec1, __m128 vec2) noexcept {
    return _mm_sub_ps(_mm_mul_ps(vec1, VecSwizzle(vec2, 3, 0, 3, 0)),
                      _mm_mul_ps(VecSwizzle(vec1, 1, 0, 3, 2), VecSwizzle(vec2, 2, 1, 2, 1)));
}

static Mat4 inverse(Mat4 m) noexcept {
    __m128 A = VecShuffle_0101(of(m.pack[0]), of(m.pack[1]));
    __m128 B = VecShuffle_2323(of(m.pack[0]), of(m.pack[1]));
    __m128 C = VecShuffle_0101(of(m.pack[2]), of(m.pack[3]));
    __m128 D = VecShuffle_2323(of(m.pack[2]), of(m.pack[3]));

    __m128 detSub = _mm_sub_ps(_mm_mul_ps(VecShuffle(of(m.pack[0]), of(m.pack[2]), 0, 2, 0, 2),
                                          VecShuffle(of(m.pack[1]), of(m.pack[3]), 1, 3, 1, 3)),
                               _mm_mul_ps(VecShuffle(of(m.pack[0]), of(m.pack[2]), 1, 3, 1, 3),
                                          VecShuffle(of(m.pack[1]), of(m.pack[3]), 0, 2, 0, 2)));
    __m128 detA = VecSwizzle1(detSub, 0);
    __m128 detB = VecSwizzle1(detSub, 1);
    __m128 detC = VecSwizzle1(detSub, 2);
    __m128 detD = VecSwizzle1(detSub, 3);
    __m128 D_C = Mat2AdjMul(D, C);
    __m128 A_B = Mat2AdjMul(A, B);
    __m128 X_ = _mm_sub_ps(_mm_mul_ps(detD, A), Mat2Mul(B, D_C));
    __m128 W_ = _mm_sub_ps(_mm_mul_ps(detA, D), Mat2Mul(C, A_B));

    __m128 detM = _mm_mul_ps(detA, detD);
    __m128 Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), Mat2MulAdj(D, A_B));
    __m128 Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), Mat2MulAdj(A, D_C));
    detM = _mm_add_ps(detM, _mm_mul_ps(detB, detC));

    __m128 tr = _mm_mul_ps(A_B, VecSwizzle(D_C, 0, 2, 1, 3));
    tr = _mm_hadd_ps(tr, tr);
    tr = _mm_hadd_ps(tr, tr);
    detM = _mm_sub_ps(detM, tr);

    const __m128 adjSignMask = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
    __m128 rDetM = _mm_div_ps(adjSignMask, detM);

    X_ = _mm_mul_ps(X_, rDetM);
    Y_ = _mm_mul_ps(Y_, rDetM);
    Z_ = _mm_mul_ps(Z_, rDetM);
    W_ = _mm_mul_ps(W_, rDetM);

    Mat4 r;
    r.pack[0] = to(VecShuffle(X_, Y_, 3, 1, 3, 1));
    r.pack[1] = to(VecShuffle(X_, Y_, 2, 0, 2, 0));
    r.pack[2] = to(VecShuffle(Z_, W_, 3, 1, 3, 1));
    r.pack[3] = to(VecShuffle(Z_, W_, 2, 0, 2, 0));
    return r;
}

} // namespace detail

Mat4 Mat4::I = Mat4{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
Mat4 Mat4::zero = Mat4{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
Mat4 Mat4::swap_x_z = Mat4{0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                           1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

Mat4 Mat4::look_at(Vec3 pos, Vec3 at, Vec3 up) noexcept {
    Mat4 ret = Mat4::zero;
    Vec3 F = normalize(at - pos);
    Vec3 S = normalize(cross(F, up));
    Vec3 U = cross(S, F);
    ret[0][0] = S.x;
    ret[0][1] = U.x;
    ret[0][2] = -F.x;
    ret[1][0] = S.y;
    ret[1][1] = U.y;
    ret[1][2] = -F.y;
    ret[2][0] = S.z;
    ret[2][1] = U.z;
    ret[2][2] = -F.z;
    ret[3][0] = -dot(S, pos);
    ret[3][1] = -dot(U, pos);
    ret[3][2] = dot(F, pos);
    ret[3][3] = 1.0f;
    return ret;
}

Mat4 Mat4::scale(Vec3 s) noexcept {
    Mat4 ret;
    ret[0][0] = s.x;
    ret[1][1] = s.y;
    ret[2][2] = s.z;
    return ret;
}

Mat4 Mat4::rotate(f32 a, Vec3 axis) noexcept {
    Mat4 ret;
    f32 c = Math::cos(Math::radians(a));
    f32 s = Math::sin(Math::radians(a));
    axis = normalize(axis);
    Vec3 temp = axis * (1.0f - c);
    ret[0][0] = c + temp[0] * axis[0];
    ret[0][1] = temp[0] * axis[1] + s * axis[2];
    ret[0][2] = temp[0] * axis[2] - s * axis[1];
    ret[1][0] = temp[1] * axis[0] - s * axis[2];
    ret[1][1] = c + temp[1] * axis[1];
    ret[1][2] = temp[1] * axis[2] + s * axis[0];
    ret[2][0] = temp[2] * axis[0] + s * axis[1];
    ret[2][1] = temp[2] * axis[1] - s * axis[0];
    ret[2][2] = c + temp[2] * axis[2];
    return ret;
}

Mat4 Mat4::translate(Vec3 v) noexcept {
    Mat4 ret;
    ret[3].xyz() = v;
    return ret;
}

Mat4 Mat4::ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) noexcept {
    Mat4 ret;
    ret[0][0] = 2.0f / (r - l);
    ret[1][1] = 2.0f / (t - b);
    ret[2][2] = 2.0f / (n - f);
    ret[3][0] = (-l - r) / (r - l);
    ret[3][1] = (-b - t) / (t - b);
    ret[3][2] = -n / (f - n);
    return ret;
}

Mat4 Mat4::proj(f32 fov, f32 ar, f32 n) noexcept {
    f32 f = 1.0f / Math::tan(Math::radians(fov) / 2.0f);
    Mat4 ret;
    ret[0][0] = f / ar;
    ret[1][1] = -f;
    ret[2][2] = 0.0f;
    ret[3][3] = 0.0f;
    ret[3][2] = n;
    ret[2][3] = -1.0f;
    return ret;
}

Mat4 Mat4::rotate_y_to(Vec3 dir) noexcept {

    dir.normalize();

    if(Math::abs(dir.y - 1.0f) < Math::EPS_F)
        return Mat4::I;
    else if(Math::abs(dir.y + 1.0f) < Math::EPS_F)
        return Mat4{Vec4{1.0f, 0.0f, 0.0f, 0.0f}, Vec4{0.0f, -1.0f, 0.0f, 0.0f},
                    Vec4{0.0f, 0.0f, 1.0f, 0.0f}, Vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    else {
        Vec3 x = cross(dir, Vec3{0.0f, 1.0f, 0.0f}).unit();
        Vec3 z = cross(x, dir).unit();
        return Mat4{Vec4{x, 0.0f}, Vec4{dir, 0.0f}, Vec4{z, 0.0f}, Vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    }
}

Mat4 Mat4::rotate_z_to(Vec3 dir) noexcept {
    Mat4 y = rotate_y_to(dir);
    Vec4 _y = y[1];
    Vec4 _z = y[2];
    y[1] = _z;
    y[2] = -_y;
    return y;
}

Mat4 Mat4::inverse(Mat4 m) noexcept {
    return detail::inverse(m);
}

Mat4 Mat4::transpose(Mat4 m) noexcept {
    Mat4 ret;
    for(u64 i = 0; i < 4; i++)
        for(u64 j = 0; j < 4; j++) ret[i][j] = m[j][i];
    return ret;
}

bool Mat4::operator==(Mat4 m) const noexcept {
    return columns[0] == m.columns[0] && columns[1] == m.columns[1] && columns[2] == m.columns[2] &&
           columns[3] == m.columns[3];
}

bool Mat4::operator!=(Mat4 m) const noexcept {
    return columns[0] != m.columns[0] || columns[1] != m.columns[1] || columns[2] != m.columns[2] ||
           columns[3] != m.columns[3];
}

Mat4 Mat4::operator+(Mat4 m) const noexcept {
    Mat4 ret;
    for(u64 i = 0; i < 4; i++) ret.pack[i] = F32x4::add(pack[i], m.pack[i]);
    return ret;
}

Mat4 Mat4::operator-(Mat4 m) const noexcept {
    Mat4 ret;
    for(u64 i = 0; i < 4; i++) ret.pack[i] = F32x4::sub(pack[i], m.pack[i]);
    return ret;
}

Mat4 Mat4::operator*(Mat4 m) const noexcept {
    Mat4 ret;
    for(u64 i = 0; i < 4; i++) {
        ret.pack[i] = F32x4::add(F32x4::add(F32x4::mul(F32x4::set1(m[i][0]), pack[0]),
                                            F32x4::mul(F32x4::set1(m[i][1]), pack[1])),
                                 F32x4::add(F32x4::mul(F32x4::set1(m[i][2]), pack[2]),
                                            F32x4::mul(F32x4::set1(m[i][3]), pack[3])));
    }
    return ret;
}

Mat4 Mat4::operator+(f32 s) const noexcept {
    Mat4 ret;
    F32x4 s4 = F32x4::set1(s);
    for(u64 i = 0; i < 4; i++) ret.pack[i] = F32x4::add(pack[i], s4);
    return ret;
}

Mat4 Mat4::operator-(f32 s) const noexcept {
    Mat4 ret;
    F32x4 s4 = F32x4::set1(s);
    for(u64 i = 0; i < 4; i++) ret.pack[i] = F32x4::sub(pack[i], s4);
    return ret;
}

Mat4 Mat4::operator*(f32 s) const noexcept {
    Mat4 ret;
    F32x4 s4 = F32x4::set1(s);
    for(u64 i = 0; i < 4; i++) ret.pack[i] = F32x4::mul(pack[i], s4);
    return ret;
}

Mat4 Mat4::operator/(f32 s) const noexcept {
    Mat4 ret;
    F32x4 s4 = F32x4::set1(s);
    for(u64 i = 0; i < 4; i++) ret.pack[i] = F32x4::div(pack[i], s4);
    return ret;
}

Vec4& Mat4::operator[](u64 idx) noexcept {
    assert(idx < 4);
    return columns[idx];
}

Vec4 Mat4::operator[](u64 idx) const noexcept {
    assert(idx < 4);
    return columns[idx];
}

Vec4 Mat4::operator*(Vec4 v) const noexcept {
    return Vec4{F32x4::add(
        F32x4::add(F32x4::mul(pack[0], F32x4::set1(v.x)), F32x4::mul(pack[1], F32x4::set1(v.y))),
        F32x4::add(F32x4::mul(pack[2], F32x4::set1(v.z)), F32x4::mul(pack[3], F32x4::set1(v.w))))};
}

Vec3 Mat4::operator*(Vec3 v) const noexcept {
    return Vec4{F32x4::add(F32x4::add(F32x4::mul(pack[0], F32x4::set1(v.x)),
                                      F32x4::mul(pack[1], F32x4::set1(v.y))),
                           F32x4::add(F32x4::mul(pack[2], F32x4::set1(v.z)), pack[3]))}
        .xyz();
}

Vec3 Mat4::rotate(Vec3 v) const noexcept {
    return Vec4{F32x4::add(F32x4::add(F32x4::mul(pack[0], F32x4::set1(v.x)),
                                      F32x4::mul(pack[1], F32x4::set1(v.y))),
                           F32x4::mul(pack[2], F32x4::set1(v.z)))}
        .xyz();
}

Mat4 Mat4::T() const noexcept {
    return transpose(*this);
}

Mat4 Mat4::inverse() const noexcept {
    return detail::inverse(*this);
}

Vec3 Mat4::to_euler() const noexcept {
    bool single = true;
    constexpr static f32 singularity[] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
                                          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0};
    for(u64 i = 0; i < 12 && single; i++) {
        single = single && Math::abs(data[i] - singularity[i]) < Math::EPS_F;
    }
    if(single) return Vec3{0.0f, 0.0f, 180.0f};

    Vec3 eul1, eul2;

    f32 cy = Math::hypot(columns[0][0], columns[0][1]);
    if(cy > Math::EPS_F) {
        eul1[0] = Math::atan2(columns[1][2], columns[2][2]);
        eul1[1] = Math::atan2(-columns[0][2], cy);
        eul1[2] = Math::atan2(columns[0][1], columns[0][0]);

        eul2[0] = Math::atan2(-columns[1][2], -columns[2][2]);
        eul2[1] = Math::atan2(-columns[0][2], -cy);
        eul2[2] = Math::atan2(-columns[0][1], -columns[0][0]);
    } else {
        eul1[0] = Math::atan2(-columns[2][1], columns[1][1]);
        eul1[1] = Math::atan2(-columns[0][2], cy);
        eul1[2] = 0;
        eul2 = eul1;
    }
    f32 d1 = Math::abs(eul1[0]) + Math::abs(eul1[1]) + Math::abs(eul1[2]);
    f32 d2 = Math::abs(eul2[0]) + Math::abs(eul2[1]) + Math::abs(eul2[2]);
    if(d1 > d2)
        return Math::degrees(eul2);
    else
        return Math::degrees(eul1);
}

} // namespace rpp::Math
