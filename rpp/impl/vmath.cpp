
#include "../vmath.h"

namespace rpp::Math {

namespace detail {

using SIMD::f32_4;

static f32_4 Mat2Mul(f32_4 vec1, f32_4 vec2) noexcept {
    return (vec1 * vec2.xwxw) + (vec1.yxwz * vec2.zyzy);
}

static f32_4 Mat2AdjMul(f32_4 vec1, f32_4 vec2) noexcept {
    return (vec1.wwxx * vec2) - (vec1.yyzz * vec2.zwxy);
}

static f32_4 Mat2MulAdj(f32_4 vec1, f32_4 vec2) noexcept {
    return (vec1 * vec2.wxwx) - (vec1.yxwz * vec2.zyzy);
}

static f32_4 hadd(f32_4 v) noexcept {
    // folllowing Intel's _mm_hadd_ps
    const float v0 = v.z + v.w;
    const float v1 = v.x + v.y;
    return {v1, v0, v1, v0};
}

static Mat4 inverse(Mat4 m) noexcept {
    const auto m_pack0 = m.pack[0].data;
    const auto m_pack1 = m.pack[1].data;
    const auto m_pack2 = m.pack[2].data;
    const auto m_pack3 = m.pack[3].data;

    const auto A = __builtin_shufflevector(m_pack0, m_pack1, 0, 1, 4, 5);
    const auto B = __builtin_shufflevector(m_pack0, m_pack1, 2, 3, 6, 7);
    const auto C = __builtin_shufflevector(m_pack2, m_pack3, 0, 1, 4, 5);
    const auto D = __builtin_shufflevector(m_pack2, m_pack3, 2, 3, 6, 7);

    const auto E = __builtin_shufflevector(m_pack0, m_pack2, 0, 2, 4, 6);
    const auto F = __builtin_shufflevector(m_pack1, m_pack3, 1, 3, 5, 7);
    const auto G = __builtin_shufflevector(m_pack0, m_pack2, 1, 3, 5, 7);
    const auto H = __builtin_shufflevector(m_pack1, m_pack3, 0, 2, 4, 6);

    const auto detSub = (E * F) - (G * H);
    const auto detA = detSub.xxxx;
    const auto detB = detSub.yyyy;
    const auto detC = detSub.zzzz;
    const auto detD = detSub.wwww;

    const auto D_C = Mat2AdjMul(D, C);
    const auto A_B = Mat2AdjMul(A, B);
    auto X_ = detD * A - Mat2Mul(B, D_C);
    auto W_ = detA * D - Mat2Mul(C, A_B);

    auto detM = detA * detD;
    auto Y_ = detB * C - Mat2MulAdj(D, A_B);
    auto Z_ = detC * B - Mat2MulAdj(A, D_C);
    detM += detB * detC;

    auto tr = A_B * D_C.xzyw;
    tr = hadd(tr);
    tr = hadd(tr);
    detM -= tr;

    const auto adjSignMask = f32_4{1.f, -1.f, -1.f, 1.f};
    const auto rDetM = adjSignMask / detM;

    X_ *= rDetM;
    Y_ *= rDetM;
    Z_ *= rDetM;
    W_ *= rDetM;

    Mat4 r;
    r.pack[0].data = __builtin_shufflevector(X_, Y_, 3, 1, 7, 5);
    r.pack[1].data = __builtin_shufflevector(X_, Y_, 2, 0, 6, 4);
    r.pack[2].data = __builtin_shufflevector(Z_, W_, 3, 1, 7, 5);
    r.pack[3].data = __builtin_shufflevector(Z_, W_, 2, 0, 6, 4);
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
