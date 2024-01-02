
#pragma once

#include "base.h"
#include "simd.h"

#ifdef RPP_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
#endif

namespace rpp {
namespace Math {

using SIMD::F32x4;

namespace detail {

template<typename T>
struct Vec2_base;
template<typename T>
struct Vec3_base;
template<typename T>
struct Vec4_base;
template<typename T, u64 N>
struct VecN_base;

template<typename T, u64 N>
using Vect_Base =
    If<N == 2, Vec2_base<T>, If<N == 3, Vec3_base<T>, If<N == 4, Vec4_base<T>, VecN_base<T, N>>>>;

template<Trivial T, u64 N>
struct Vect : Vect_Base<T, N> {

    static_assert(N > 1);
    using Base = Vect_Base<T, N>;

    static constexpr bool is_float = Float<T>;
    static constexpr bool is_simd = Same<T, f32> && N == 4;

    constexpr Vect() : Base{} {
        if constexpr(is_simd)
            this->pack = F32x4::zero();
        else
            for(u64 i = 0; i < N; i++) this->data[i] = T{};
    };

    constexpr explicit Vect(F32x4 p)
        requires is_simd
        : Base{} {
        this->pack = p;
    }

    constexpr explicit Vect(T x) : Base{} {
        for(u64 i = 0; i < N; i++) this->data[i] = x;
    }

    constexpr explicit Vect(Vect<T, N - 1> f, T s)
        requires(N > 2)
        : Base{} {
        for(u64 i = 0; i < N - 1; i++) this->data[i] = f[i];
        this->data[N - 1] = s;
    }

    template<typename... S>
        requires All_Are<T, S...> && Length<N, S...>
    constexpr explicit Vect(S... args) : Base{args...} {
    }

    ~Vect() = default;
    Vect(const Vect&) = default;
    Vect& operator=(const Vect&) = default;
    Vect(Vect&&) = default;
    Vect& operator=(Vect&&) = default;

    T& operator[](u64 idx) {
        return this->data[idx];
    }
    const T& operator[](u64 idx) const {
        return this->data[idx];
    }

    Vect operator+=(Vect v) {
        if constexpr(is_simd)
            this->pack = F32x4::add(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] += v.data[i];
        return *this;
    }
    Vect operator-=(Vect v) {
        if constexpr(is_simd)
            this->pack = F32x4::sub(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] -= v.data[i];
        return *this;
    }
    Vect operator*=(Vect v) {
        if constexpr(is_simd)
            this->pack = F32x4::mul(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] *= v.data[i];
        return *this;
    }
    Vect operator/=(Vect v) {
        if constexpr(is_simd)
            this->pack = F32x4::div(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] /= v.data[i];
        return *this;
    }

    Vect operator+=(T s) {
        if constexpr(is_simd)
            this->pack = F32x4::add(this->pack, F32x4::set1(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] += s;
        return *this;
    }
    Vect operator-=(T s) {
        if constexpr(is_simd)
            this->pack = F32x4::sub(this->pack, F32x4::set1(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] -= s;
        return *this;
    }
    Vect operator*=(T s) {
        if constexpr(is_simd)
            this->pack = F32x4::mul(this->pack, F32x4::set1(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] *= s;
        return *this;
    }
    Vect operator/=(T s) {
        if constexpr(is_simd)
            this->pack = F32x4::div(this->pack, F32x4::set1(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] /= s;
        return *this;
    }

    Vect operator+(Vect o) const {
        if constexpr(is_simd)
            return Vect{F32x4::add(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] + o.data[i];
            return r;
        }
    }
    Vect operator-(Vect o) const {
        if constexpr(is_simd)
            return Vect{F32x4::sub(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] - o.data[i];
            return r;
        }
    }
    Vect operator*(Vect o) const {
        if constexpr(is_simd)
            return Vect{F32x4::mul(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] * o.data[i];
            return r;
        }
    }
    Vect operator/(Vect o) const {
        if constexpr(is_simd)
            return Vect{F32x4::div(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] / o.data[i];
            return r;
        }
    }

    Vect operator+(T s) const {
        if constexpr(is_simd)
            return Vect{F32x4::add(this->pack, F32x4::set1(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] + s;
            return r;
        }
    }
    Vect operator-(T s) const {
        if constexpr(is_simd)
            return Vect{F32x4::sub(this->pack, F32x4::set1(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] - s;
            return r;
        }
    }
    Vect operator*(T s) const {
        if constexpr(is_simd)
            return Vect{F32x4::mul(this->pack, F32x4::set1(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] * s;
            return r;
        }
    }
    Vect operator/(T s) const {
        if constexpr(is_simd)
            return Vect{F32x4::div(this->pack, F32x4::set1(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] / s;
            return r;
        }
    }

    bool operator==(Vect o) const {
        if constexpr(is_simd)
            return F32x4::movemask(F32x4::cmpeq(this->pack, o.pack)) == 0xf;
        else {
            for(u64 i = 0; i < N; i++)
                if(this->data[i] != o.data[i]) return false;
            return true;
        }
    }
    bool operator!=(Vect o) const {
        if constexpr(is_simd)
            return F32x4::movemask(F32x4::cmpeq(this->pack, o.pack)) != 0xf;
        else {
            for(u64 i = 0; i < N; i++)
                if(this->data[i] != o.data[i]) return true;
            return false;
        }
    }

    Vect abs() const
        requires is_float || Signed_Int<T>
    {
        Vect r;
        if constexpr(is_simd)
            r.pack = F32x4::abs(this->pack);
        else
            for(u64 i = 0; i < N; i++) r.data[i] = Math::abs(this->data[i]);
        return r;
    }
    Vect operator-()
        requires is_float || Signed_Int<T>
    {
        if constexpr(is_simd)
            this->pack = F32x4::sub(F32x4::zero(), this->pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] = -this->data[i];
        return *this;
    }

    Vect normalize()
        requires is_float
    {
        T l = norm();
        if constexpr(is_simd)
            this->pack = F32x4::div(this->pack, F32x4::set1(l));
        else
            for(u64 i = 0; i < N; i++) this->data[i] /= l;
        return *this;
    }

    Vect unit() const
        requires is_float
    {
        Vect r;
        T l = norm();
        if constexpr(is_simd)
            r.pack = F32x4::div(this->pack, F32x4::set1(l));
        else
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] / l;
        return r;
    }

    Vect<T, N - 1> proj() const
        requires(N > 2)
    {
        Vect<T, N - 1> r;
        for(u64 i = 0; i < N - 1; i++) r.data[i] = this->data[i] / this->data[N - 1];
        return r;
    }

    template<typename U>
    Vect<U, N> as() const {
        Vect<U, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = static_cast<U>(this->data[i]);
        return r;
    }

    T norm2() const {
        if constexpr(is_simd)
            return F32x4::dp(this->pack, this->pack);
        else {
            T r = T{};
            for(u64 i = 0; i < N; i++) r += this->data[i] * this->data[i];
            return r;
        }
    }

    T min() const {
        T r = this->data[0];
        for(u64 i = 1; i < N; i++) r = Math::min(r, this->data[i]);
        return r;
    }

    T max() const {
        T r = this->data[0];
        for(u64 i = 1; i < N; i++) r = Math::max(r, this->data[i]);
        return r;
    }

    Vect<T, N> floor()
        requires is_float
    {
        Vect<T, N> r;
        if constexpr(is_simd)
            r.pack = F32x4::floor(this->pack);
        else
            for(u64 i = 0; i < N; i++) r.data[i] = Math::floor(this->data[i]);
        return r;
    }

    Vect<T, N> ceil()
        requires is_float
    {
        Vect<T, N> r;
        if constexpr(is_simd)
            r.pack = F32x4::ceil(this->pack);
        else
            for(u64 i = 0; i < N; i++) r.data[i] = Math::ceil(this->data[i]);
        return r;
    }

    T norm() const
        requires is_float
    {
        return Math::sqrt(norm2());
    }

    const T* begin() const {
        return this->data;
    }
    const T* end() const {
        return this->data + N;
    }
    T* begin() {
        return this->data;
    }
    T* end() {
        return this->data + N;
    }
};

template<typename T>
struct Vec2_base {
    union {
        struct {
            T x, y;
        };
        T data[2];
    };
};
template<typename T>
struct Vec3_base {
    union {
        struct {
            T x, y, z;
        };
        T data[3];
    };

    Vect<T, 2>& xy() {
        return *reinterpret_cast<Vect<T, 2>*>(&x);
    }
    Vect<T, 2>& yz() {
        return *reinterpret_cast<Vect<T, 2>*>(&y);
    }
    const Vect<T, 2>& xy() const {
        return *reinterpret_cast<const Vect<T, 2>*>(&x);
    }
    const Vect<T, 2>& yz() const {
        return *reinterpret_cast<const Vect<T, 2>*>(&y);
    }
};
template<typename T>
struct Vec4_base {
    union {
        struct {
            T x, y, z, w;
        };
        T data[4];
    };

    Vect<T, 2>& xy() {
        return *reinterpret_cast<Vect<T, 2>*>(&x);
    }
    Vect<T, 2>& yz() {
        return *reinterpret_cast<Vect<T, 2>*>(&y);
    }
    Vect<T, 2>& zw() {
        return *reinterpret_cast<Vect<T, 2>*>(&z);
    }
    const Vect<T, 2>& xy() const {
        return *reinterpret_cast<const Vect<T, 2>*>(&x);
    }
    const Vect<T, 2>& yz() const {
        return *reinterpret_cast<const Vect<T, 2>*>(&y);
    }
    const Vect<T, 2>& zw() const {
        return *reinterpret_cast<const Vect<T, 2>*>(&z);
    }

    Vect<T, 3>& xyz() {
        return *reinterpret_cast<Vect<T, 3>*>(&x);
    }
    Vect<T, 3>& yzw() {
        return *reinterpret_cast<Vect<T, 3>*>(&y);
    }
    const Vect<T, 3>& xyz() const {
        return *reinterpret_cast<const Vect<T, 3>*>(&x);
    }
    const Vect<T, 3>& yzw() const {
        return *reinterpret_cast<const Vect<T, 3>*>(&y);
    }
};
template<>
struct Vec4_base<f32> {
    union {
        struct {
            f32 x, y, z, w;
        };
        f32 data[4];
        SIMD::F32x4 pack;
    };

    Vect<f32, 2>& xy() {
        return *reinterpret_cast<Vect<f32, 2>*>(&x);
    }
    Vect<f32, 2>& yz() {
        return *reinterpret_cast<Vect<f32, 2>*>(&y);
    }
    Vect<f32, 2>& zw() {
        return *reinterpret_cast<Vect<f32, 2>*>(&z);
    }
    const Vect<f32, 2>& xy() const {
        return *reinterpret_cast<const Vect<f32, 2>*>(&x);
    }
    const Vect<f32, 2>& yz() const {
        return *reinterpret_cast<const Vect<f32, 2>*>(&y);
    }
    const Vect<f32, 2>& zw() const {
        return *reinterpret_cast<const Vect<f32, 2>*>(&z);
    }

    Vect<f32, 3>& xyz() {
        return *reinterpret_cast<Vect<f32, 3>*>(&x);
    }
    Vect<f32, 3>& yzw() {
        return *reinterpret_cast<Vect<f32, 3>*>(&y);
    }
    const Vect<f32, 3>& xyz() const {
        return *reinterpret_cast<const Vect<f32, 3>*>(&x);
    }
    const Vect<f32, 3>& yzw() const {
        return *reinterpret_cast<const Vect<f32, 3>*>(&y);
    }
};
template<typename T, u64 N>
struct VecN_base {
    T data[N];
};

} // namespace detail

using detail::Vect;

using Vec2 = Vect<f32, 2>;
using Vec3 = Vect<f32, 3>;
using Vec4 = Vect<f32, 4>;
template<u64 N>
using VecN = Vect<f32, N>;

using Vec2i = Vect<i32, 2>;
using Vec3i = Vect<i32, 3>;
using Vec4i = Vect<i32, 4>;
template<u64 N>
using VecNi = Vect<i32, N>;

using Vec2u = Vect<u32, 2>;
using Vec3u = Vect<u32, 3>;
using Vec4u = Vect<u32, 4>;
template<u64 N>
using VecNu = Vect<u32, N>;

inline Vec3 cross(Vec3 l, Vec3 r) {
    return Vec3{l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x};
}

template<typename T, u64 N>
Vect<T, N> min(Vect<T, N> x, Vect<T, N> y) {
    if constexpr(Vect<T, N>::is_simd)
        return Vect<T, N>{F32x4::min(x.pack, y.pack)};
    else {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = Math::min(x.data[i], y.data[i]);
        return r;
    }
}

template<typename T, u64 N>
Vect<T, N> max(Vect<T, N> x, Vect<T, N> y) {
    if constexpr(Vect<T, N>::is_simd)
        return Vect<T, N>{F32x4::max(x.pack, y.pack)};
    else {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = Math::max(x.data[i], y.data[i]);
        return r;
    }
}

template<typename T, u64 N>
Vect<T, N> abs(Vect<T, N> x)
    requires Float<T> || Signed_Int<T>
{
    if constexpr(Vect<T, N>::is_simd)
        return Vect<T, N>{F32x4::abs(x.pack)};
    else {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = Math::abs(x.data[i]);
        return r;
    }
}

template<typename T, u64 N>
T dot(Vect<T, N> x, Vect<T, N> y) {
    if constexpr(Vect<T, N>::is_simd)
        return F32x4::dp(x.pack, y.pack);
    else {
        T r = {};
        for(u64 i = 0; i < N; i++) r += x.data[i] * y.data[i];
        return r;
    }
}

template<Float T, u64 N>
Vect<T, N> lerp(Vect<T, N> min, Vect<T, N> max, T dist) {
    return min + (max - min) * dist;
}

template<typename T, u64 N>
Vect<T, N> clamp(Vect<T, N> x, Vect<T, N> min, Vect<T, N> max) {
    return Math::max(Math::min(x, max), min);
}

template<Float T, u64 N>
Vect<T, N> normalize(Vect<T, N> x) {
    return x.unit();
}

struct Mat4 {

    union {
        f32 data[16];
        F32x4 pack[4];
        Vec4 columns[4];
    };

    explicit Mat4()
        : columns{Vec4{1.0f, 0.0f, 0.0f, 0.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f},
                  Vec4{0.0f, 0.0f, 1.0f, 0.0f}, Vec4{0.0f, 0.0f, 0.0f, 1.0f}} {
    }
    explicit Mat4(Vec4 x, Vec4 y, Vec4 z, Vec4 w) : columns{x, y, z, w} {
    }

    template<typename... Ts>
        requires Length<16, Ts...> && All_Are<f32, Ts...>
    explicit Mat4(Ts... args) : data{args...} {
    }

    ~Mat4() = default;
    Mat4(const Mat4&) = default;
    Mat4& operator=(const Mat4&) = default;
    Mat4(Mat4&&) = default;
    Mat4& operator=(Mat4&&) = default;

    static Mat4 look_at(Vec3 pos, Vec3 at, Vec3 up);
    static Mat4 scale(Vec3 s);
    static Mat4 rotate(f32 a, Vec3 axis);
    static Mat4 translate(Vec3 v);
    static Mat4 ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
    static Mat4 proj(f32 fov, f32 ar, f32 n);
    static Mat4 rotate_y_to(Vec3 dir);
    static Mat4 rotate_z_to(Vec3 dir);
    static Mat4 inverse(Mat4 m);
    static Mat4 transpose(Mat4 m);

    bool operator==(Mat4 m) const;
    bool operator!=(Mat4 m) const;

    Mat4 operator+(Mat4 m) const;
    Mat4 operator-(Mat4 m) const;
    Mat4 operator*(Mat4 m) const;

    Mat4 operator+(f32 s) const;
    Mat4 operator-(f32 s) const;
    Mat4 operator*(f32 s) const;
    Mat4 operator/(f32 s) const;

    Vec4& operator[](u64 idx);
    Vec4 operator[](u64 idx) const;

    Vec4 operator*(Vec4 v) const;
    Vec3 operator*(Vec3 v) const;
    Vec3 rotate(Vec3 v) const;

    Mat4 T() const;
    Mat4 inverse() const;
    Vec3 to_euler() const;

    const Vec4* begin() const;
    const Vec4* end() const;
    Vec4* begin();
    Vec4* end();

    static Mat4 I;
    static Mat4 zero;
    static Mat4 swap_x_z;
};

struct Quat : detail::Vect_Base<f32, 4> {

    using Base = detail::Vect_Base<f32, 4>;

    explicit Quat() : Base{0.0f, 0.0f, 0.0f, 1.0f} {
    }
    explicit Quat(f32 x, f32 y, f32 z, f32 w) : Base{x, y, z, w} {
    }
    explicit Quat(Vec3 complex, f32 real) : Base{complex.x, complex.y, complex.z, real} {
    }
    explicit Quat(Vec4 src) : Base{src} {
    }

    ~Quat() = default;
    Quat(const Quat&) = default;
    Quat& operator=(const Quat&) = default;
    Quat(Quat&&) = default;
    Quat& operator=(Quat&&) = default;

    static Quat axis_angle(Vec3 axis, f32 angle) {
        axis.normalize();
        angle = Math::radians(angle) / 2.0f;
        f32 sin = Math::sin(angle);
        f32 x = sin * axis.x;
        f32 y = sin * axis.y;
        f32 z = sin * axis.z;
        f32 w = Math::cos(angle);
        return Quat(x, y, z, w).unit();
    }
    static Quat euler(Vec3 angles) {
        if(angles == Vec3{0.0f, 0.0f, 180.0f} || angles == Vec3{180.0f, 0.0f, 0.0f})
            return Quat{0.0f, 0.0f, -1.0f, 0.0f};
        f32 c1 = Math::cos(Math::radians(angles[2] * 0.5f));
        f32 c2 = Math::cos(Math::radians(angles[1] * 0.5f));
        f32 c3 = Math::cos(Math::radians(angles[0] * 0.5f));
        f32 s1 = Math::sin(Math::radians(angles[2] * 0.5f));
        f32 s2 = Math::sin(Math::radians(angles[1] * 0.5f));
        f32 s3 = Math::sin(Math::radians(angles[0] * 0.5f));
        f32 x = c1 * c2 * s3 - s1 * s2 * c3;
        f32 y = c1 * s2 * c3 + s1 * c2 * s3;
        f32 z = s1 * c2 * c3 - c1 * s2 * s3;
        f32 w = c1 * c2 * c3 + s1 * s2 * s3;
        return Quat(x, y, z, w);
    }

    f32& operator[](int idx) {
        return data[idx];
    }
    f32 operator[](int idx) const {
        return data[idx];
    }

    Quat conjugate() const {
        return Quat(-x, -y, -z, w);
    }
    Quat inverse() const {
        return conjugate().unit();
    }
    Vec3 complex() const {
        return Vec3(x, y, z);
    }
    f32 real() const {
        return w;
    }

    f32 norm2() const {
        return x * x + y * y + z * z + w * w;
    }
    f32 norm() const {
        return Math::sqrt(norm2());
    }
    Quat unit() const {
        f32 n = norm();
        return Quat(x / n, y / n, z / n, w / n);
    }

    Quat operator*(const Quat& r) const {
        return Quat(y * r.z - z * r.y + x * r.w + w * r.x, z * r.x - x * r.z + y * r.w + w * r.y,
                    x * r.y - y * r.x + z * r.w + w * r.z, w * r.w - x * r.x - y * r.y - z * r.z);
    }
    Quat operator*(f32 s) const {
        return Quat(s * x, s * y, s * z, s * w);
    }

    Quat operator+(const Quat& r) const {
        return Quat(x + r.x, y + r.y, z + r.z, w + r.w);
    }
    Quat operator-(const Quat& r) const {
        return Quat(x - r.x, y - r.y, z - r.z, w - r.w);
    }
    Quat operator-() const {
        return Quat(-x, -y, -z, -w);
    }

    Vec3 to_euler() const {
        return unit().to_mat().to_euler();
    }

    Mat4 to_mat() const {
        return Mat4{
            Vec4{1 - 2 * y * y - 2 * z * z, 2 * x * y + 2 * z * w, 2 * x * z - 2 * y * w, 0.0f},
            Vec4{2 * x * y - 2 * z * w, 1 - 2 * x * x - 2 * z * z, 2 * y * z + 2 * x * w, 0.0f},
            Vec4{2 * x * z + 2 * y * w, 2 * y * z - 2 * x * w, 1 - 2 * x * x - 2 * y * y, 0.0f},
            Vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    }

    Vec3 rotate(Vec3 v) const {
        return (((*this) * Quat(v, 0)) * conjugate()).complex();
    }

    bool operator==(const Quat& v) const {
        return x == v.x && y == v.y && z == v.z && w == v.w;
    }
    bool operator!=(const Quat& v) const {
        return x != v.x || y != v.y || z != v.z || w != v.w;
    }

    const f32* begin() const {
        return this->data;
    }
    const f32* end() const {
        return this->data + 4;
    }
    f32* begin() {
        return this->data;
    }
    f32* end() {
        return this->data + 4;
    }
};

struct BBox {

    explicit BBox() : min(Limits<f32>::max()), max(Limits<f32>::min()) {
    }
    explicit BBox(Vec3 min, Vec3 max) : min(min), max(max) {
    }
    ~BBox() = default;

    explicit BBox(const BBox&) = default;
    BBox& operator=(const BBox&) = default;

    BBox(BBox&&) = default;
    BBox& operator=(BBox&&) = default;

    void reset() {
        min = Vec3(Limits<f32>::max());
        max = Vec3(Limits<f32>::min());
    }

    void enclose(Vec3 point) {
        min = Math::min(min, point);
        max = Math::max(max, point);
    }
    void enclose(BBox box) {
        min = Math::min(min, box.min);
        max = Math::max(max, box.max);
    }

    Vec3 center() const {
        return (min + max) * 0.5f;
    }

    bool empty() const {
        return min.x > max.x || min.y > max.y || min.z > max.z;
    }

    f32 surface_area() const {
        if(empty()) return 0.0f;
        Vec3 extent = max - min;
        return 2.0f * (extent.x * extent.z + extent.x * extent.y + extent.y * extent.z);
    }

    void transform(const Mat4& T) {
        Vec3 amin = min, amax = max;
        min = max = Vec3{T[3].x, T[3].y, T[3].z};
        for(u64 i = 0; i < 3; i++) {
            for(u64 j = 0; j < 3; j++) {
                f32 a = T[j][i] * amin[j];
                f32 b = T[j][i] * amax[j];
                if(a < b) {
                    min[i] += a;
                    max[i] += b;
                } else {
                    min[i] += b;
                    max[i] += a;
                }
            }
        }
    }

    void project(const Mat4& proj, Vec2& min_out, Vec2& max_out) const {

        min_out = Vec2(Limits<f32>::max());
        max_out = Vec2(Limits<f32>::min());

        Vec3 c[] = {Vec3(min.x, min.y, min.z), Vec3(max.x, min.y, min.z), Vec3(min.x, max.y, min.z),
                    Vec3(min.x, min.y, max.z), Vec3(max.x, max.y, min.z), Vec3(min.x, max.y, max.z),
                    Vec3(max.x, min.y, max.z), Vec3(max.x, max.y, max.z)};

        bool partially_behind = false, all_behind = true;
        for(auto& v : c) {
            Vec3 p = proj * v;
            if(p.z < 0.0f) {
                partially_behind = true;
            } else {
                all_behind = false;
            }
            min_out = Math::min(min_out, Vec2(p.x, p.y));
            max_out = Math::max(max_out, Vec2(p.x, p.y));
        }

        if(partially_behind && !all_behind) {
            min_out = Vec2(-1.0f, -1.0f);
            max_out = Vec2(1.0f, 1.0f);
        } else if(all_behind) {
            min_out = Vec2(0.0f, 0.0f);
            max_out = Vec2(0.0f, 0.0f);
        }
    }

    Vec3 min, max;
};

} // namespace Math

using Math::Vec2;
using Math::Vec3;
using Math::Vec4;
using Math::VecN;

using Math::Vec2i;
using Math::Vec3i;
using Math::Vec4i;
using Math::VecNi;

using Math::Vec2u;
using Math::Vec3u;
using Math::Vec4u;
using Math::VecNu;

using Math::BBox;
using Math::Mat4;
using Math::Quat;

template<typename T, u64 N>
Math::Vect<T, N> operator+(T s, Math::Vect<T, N> v) {
    if constexpr(Math::Vect<T, N>::is_simd)
        return Math::Vect<T, N>{SIMD::F32x4::add(v.pack, SIMD::F32x4::set1(s))};
    else {
        Math::Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = v.data[i] + s;
        return r;
    }
}

template<typename T, u64 N>
Math::Vect<T, N> operator-(T s, Math::Vect<T, N> v) {
    if constexpr(Math::Vect<T, N>::is_simd)
        return Math::Vect<T, N>{SIMD::F32x4::sub(v.pack, SIMD::F32x4::set1(s))};
    else {
        Math::Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = s - v.data[i];
        return r;
    }
}

template<typename T, u64 N>
Math::Vect<T, N> operator*(T s, Math::Vect<T, N> v) {
    if constexpr(Math::Vect<T, N>::is_simd)
        return Math::Vect<T, N>{SIMD::F32x4::mul(v.pack, SIMD::F32x4::set1(s))};
    else {
        Math::Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = v.data[i] * s;
        return r;
    }
}

template<typename T, u64 N>
Math::Vect<T, N> operator/(T s, Math::Vect<T, N> v) {
    if constexpr(Math::Vect<T, N>::is_simd)
        return Math::Vect<T, N>{SIMD::F32x4::div(v.pack, SIMD::F32x4::set1(s))};
    else {
        Math::Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = s / v.data[i];
        return r;
    }
}

RPP_RECORD(Vec2, RPP_FIELD(x), RPP_FIELD(y));
RPP_RECORD(Vec3, RPP_FIELD(x), RPP_FIELD(y), RPP_FIELD(z));
RPP_RECORD(Vec4, RPP_FIELD(x), RPP_FIELD(y), RPP_FIELD(z), RPP_FIELD(w));
template<u64 N>
RPP_TEMPLATE_RECORD(VecN, N, RPP_FIELD(data));

RPP_RECORD(Vec2i, RPP_FIELD(x), RPP_FIELD(y));
RPP_RECORD(Vec3i, RPP_FIELD(x), RPP_FIELD(y), RPP_FIELD(z));
RPP_RECORD(Vec4i, RPP_FIELD(x), RPP_FIELD(y), RPP_FIELD(z), RPP_FIELD(w));
template<u64 N>
RPP_TEMPLATE_RECORD(VecNi, N, RPP_FIELD(data));

RPP_RECORD(Vec2u, RPP_FIELD(x), RPP_FIELD(y));
RPP_RECORD(Vec3u, RPP_FIELD(x), RPP_FIELD(y), RPP_FIELD(z));
RPP_RECORD(Vec4u, RPP_FIELD(x), RPP_FIELD(y), RPP_FIELD(z), RPP_FIELD(w));
template<u64 N>
RPP_TEMPLATE_RECORD(VecNu, N, RPP_FIELD(data));

RPP_RECORD(Mat4, RPP_FIELD(columns));
RPP_RECORD(BBox, RPP_FIELD(min), RPP_FIELD(max));
RPP_RECORD(Quat, RPP_FIELD(x), RPP_FIELD(y), RPP_FIELD(z), RPP_FIELD(w));

namespace Hash {

template<typename T, u64 N>
struct Hash<Math::Vect<T, N>> {
    static u64 hash(const Math::Vect<T, N>& v) {
        u64 h = 0;
        for(u64 i = 0; i < N; i++) h = hash_combine(h, rpp::hash(v[i]));
        return h;
    }
};

} // namespace Hash

namespace Format {

template<Float F, u64 N>
struct Measure<Math::Vect<F, N>> {
    static u64 measure(const Math::Vect<F, N>& vect) {
        u64 length = 5;
        length += Measure<u64>::measure(N);
        for(u64 i = 0; i < N; i++) {
            length += Measure<F>::measure(vect[i]);
            if(i + 1 < N) length += 2;
        }
        return length;
    }
};
template<Int I, u64 N>
struct Measure<Math::Vect<I, N>> {
    static u64 measure(const Math::Vect<I, N>& vect) {
        u64 length = 6;
        length += Measure<u64>::measure(N);
        for(u64 i = 0; i < N; i++) {
            length += Measure<I>::measure(vect[i]);
            if(i + 1 < N) length += 2;
        }
        return length;
    }
};
template<>
struct Measure<Math::Mat4> {
    static u64 measure(const Math::Mat4& mat) {
        u64 length = 6;
        for(u64 i = 0; i < 4; i++) {
            length += 1;
            for(u64 j = 0; j < 4; j++) {
                length += Measure<f32>::measure(mat[i][j]);
                if(j + 1 < 4) length += 2;
            }
            length += 1;
            if(i + 1 < 4) length += 2;
        }
        return length;
    }
};
template<>
struct Measure<Math::Quat> {
    static u64 measure(const Math::Quat& quat) {
        u64 length = 12;
        length += Measure<f32>::measure(quat.x);
        length += Measure<f32>::measure(quat.y);
        length += Measure<f32>::measure(quat.z);
        length += Measure<f32>::measure(quat.w);
        return length;
    }
};
template<>
struct Measure<Math::BBox> {
    static u64 measure(const Math::BBox& bbox) {
        u64 length = 20;
        length += Measure<f32>::measure(bbox.min.x);
        length += Measure<f32>::measure(bbox.min.y);
        length += Measure<f32>::measure(bbox.min.z);
        length += Measure<f32>::measure(bbox.max.x);
        length += Measure<f32>::measure(bbox.max.y);
        length += Measure<f32>::measure(bbox.max.z);
        return length;
    }
};

template<Allocator O, Float F, u64 N>
struct Write<O, Math::Vect<F, N>> {
    static u64 write(String<O>& output, u64 idx, const Math::Vect<F, N>& vect) {
        idx = output.write(idx, "Vec"_v);
        idx = Write<O, u64>::write(output, idx, N);
        idx = output.write(idx, '{');
        for(u64 i = 0; i < N; i++) {
            idx = Write<O, F>::write(output, idx, vect[i]);
            if(i + 1 < N) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, '}');
    }
};
template<Allocator O, Signed_Int I, u64 N>
struct Write<O, Math::Vect<I, N>> {
    static u64 write(String<O>& output, u64 idx, const Math::Vect<I, N>& vect) {
        idx = output.write(idx, "Vec"_v);
        idx = Write<O, u64>::write(output, idx, N);
        idx = output.write(idx, "i{"_v);
        for(u64 i = 0; i < N; i++) {
            idx = Write<O, I>::write(output, idx, vect[i]);
            if(i + 1 < N) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, '}');
    }
};
template<Allocator O, Unsigned_Int I, u64 N>
struct Write<O, Math::Vect<I, N>> {
    static u64 write(String<O>& output, u64 idx, const Math::Vect<I, N>& vect) {
        idx = output.write(idx, "Vec"_v);
        idx = Write<O, u64>::write(output, idx, N);
        idx = output.write(idx, "u{"_v);
        for(u64 i = 0; i < N; i++) {
            idx = Write<O, I>::write(output, idx, vect[i]);
            if(i + 1 < N) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, '}');
    }
};
template<Allocator O>
struct Write<O, Math::Mat4> {
    static u64 write(String<O>& output, u64 idx, const Math::Mat4& mat) {
        idx = output.write(idx, "Mat4{"_v);
        for(u64 i = 0; i < 4; i++) {
            idx = output.write(idx, '{');
            for(u64 j = 0; j < 4; j++) {
                idx = Write<O, f32>::write(output, idx, mat[i][j]);
                if(j + 1 < 4) idx = output.write(idx, ", "_v);
            }
            idx = output.write(idx, '}');
            if(i + 1 < 4) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, '}');
    }
};
template<Allocator O>
struct Write<O, Math::Quat> {
    static u64 write(String<O>& output, u64 idx, const Math::Quat& quat) {
        idx = output.write(idx, "Quat{"_v);
        idx = Write<O, f32>::write(output, idx, quat.x);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, quat.y);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, quat.z);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, quat.w);
        return output.write(idx, '}');
    }
};
template<Allocator O>
struct Write<O, Math::BBox> {
    static u64 write(String<O>& output, u64 idx, const Math::BBox& bbox) {
        idx = output.write(idx, "BBox{{"_v);
        idx = Write<O, f32>::write(output, idx, bbox.min.x);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, bbox.min.y);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, bbox.min.z);
        idx = output.write(idx, "}, {"_v);
        idx = Write<O, f32>::write(output, idx, bbox.max.x);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, bbox.max.y);
        idx = output.write(idx, ", "_v);
        idx = Write<O, f32>::write(output, idx, bbox.max.z);
        return output.write(idx, "}}"_v);
    }
};

} // namespace Format

} // namespace rpp

#ifdef RPP_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
