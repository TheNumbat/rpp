
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

    constexpr static bool is_float = Float<T>;
    constexpr static bool is_simd = Same<T, f32> && N == 4;

    constexpr Vect() noexcept : Base{} {
        if constexpr(is_simd)
            this->pack = F32x4::zero();
        else
            for(u64 i = 0; i < N; i++) this->data[i] = T{};
    };

    constexpr explicit Vect(F32x4 p) noexcept
        requires is_simd
        : Base{} {
        this->pack = p;
    }

    constexpr explicit Vect(T x) noexcept : Base{} {
        for(u64 i = 0; i < N; i++) this->data[i] = x;
    }

    constexpr explicit Vect(Vect<T, N - 1> f, T s) noexcept
        requires(N > 2)
        : Base{} {
        for(u64 i = 0; i < N - 1; i++) this->data[i] = f[i];
        this->data[N - 1] = s;
    }

    template<typename... S>
        requires All_Are<T, S...> && Length<N, S...>
    constexpr explicit Vect(S... args) noexcept : Base{args...} {
    }

    constexpr ~Vect() noexcept = default;

    constexpr Vect(const Vect&) noexcept = default;
    constexpr Vect& operator=(const Vect&) noexcept = default;
    constexpr Vect(Vect&&) noexcept = default;
    constexpr Vect& operator=(Vect&&) noexcept = default;

    constexpr T& operator[](u64 idx) noexcept {
        return this->data[idx];
    }
    constexpr const T& operator[](u64 idx) const noexcept {
        return this->data[idx];
    }

    constexpr Vect operator+=(Vect v) noexcept {
        if constexpr(is_simd)
            this->pack = F32x4::add(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] += v.data[i];
        return *this;
    }
    constexpr Vect operator-=(Vect v) noexcept {
        if constexpr(is_simd)
            this->pack = F32x4::sub(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] -= v.data[i];
        return *this;
    }
    constexpr Vect operator*=(Vect v) noexcept {
        if constexpr(is_simd)
            this->pack = F32x4::mul(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] *= v.data[i];
        return *this;
    }
    constexpr Vect operator/=(Vect v) noexcept {
        if constexpr(is_simd)
            this->pack = F32x4::div(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] /= v.data[i];
        return *this;
    }

    constexpr Vect operator+=(T s) noexcept {
        if constexpr(is_simd)
            this->pack = F32x4::add(this->pack, F32x4::set1(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] += s;
        return *this;
    }
    constexpr Vect operator-=(T s) noexcept {
        if constexpr(is_simd)
            this->pack = F32x4::sub(this->pack, F32x4::set1(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] -= s;
        return *this;
    }
    constexpr Vect operator*=(T s) noexcept {
        if constexpr(is_simd)
            this->pack = F32x4::mul(this->pack, F32x4::set1(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] *= s;
        return *this;
    }
    constexpr Vect operator/=(T s) noexcept {
        if constexpr(is_simd)
            this->pack = F32x4::div(this->pack, F32x4::set1(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] /= s;
        return *this;
    }

    constexpr Vect operator+(Vect o) const noexcept {
        if constexpr(is_simd)
            return Vect{F32x4::add(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] + o.data[i];
            return r;
        }
    }
    constexpr Vect operator-(Vect o) const noexcept {
        if constexpr(is_simd)
            return Vect{F32x4::sub(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] - o.data[i];
            return r;
        }
    }
    constexpr Vect operator*(Vect o) const noexcept {
        if constexpr(is_simd)
            return Vect{F32x4::mul(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] * o.data[i];
            return r;
        }
    }
    constexpr Vect operator/(Vect o) const noexcept {
        if constexpr(is_simd)
            return Vect{F32x4::div(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] / o.data[i];
            return r;
        }
    }

    constexpr Vect operator+(T s) const noexcept {
        if constexpr(is_simd)
            return Vect{F32x4::add(this->pack, F32x4::set1(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] + s;
            return r;
        }
    }
    constexpr Vect operator-(T s) const noexcept {
        if constexpr(is_simd)
            return Vect{F32x4::sub(this->pack, F32x4::set1(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] - s;
            return r;
        }
    }
    constexpr Vect operator*(T s) const noexcept {
        if constexpr(is_simd)
            return Vect{F32x4::mul(this->pack, F32x4::set1(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] * s;
            return r;
        }
    }
    constexpr Vect operator/(T s) const noexcept {
        if constexpr(is_simd)
            return Vect{F32x4::div(this->pack, F32x4::set1(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] / s;
            return r;
        }
    }

    constexpr bool operator==(Vect o) const noexcept {
        if constexpr(is_simd)
            return static_cast<bool>(F32x4::cmpeq(this->pack, o.pack));
        else {
            for(u64 i = 0; i < N; i++)
                if(this->data[i] != o.data[i]) return false;
            return true;
        }
    }
    constexpr bool operator!=(Vect o) const noexcept {
        if constexpr(is_simd)
            return !static_cast<bool>(F32x4::cmpeq(this->pack, o.pack));
        else {
            for(u64 i = 0; i < N; i++)
                if(this->data[i] != o.data[i]) return true;
            return false;
        }
    }

    constexpr Vect abs() const noexcept
        requires is_float || Signed_Int<T>
    {
        Vect r;
        if constexpr(is_simd)
            r.pack = F32x4::abs(this->pack);
        else
            for(u64 i = 0; i < N; i++) r.data[i] = Math::abs(this->data[i]);
        return r;
    }
    constexpr Vect operator-() noexcept
        requires is_float || Signed_Int<T>
    {
        if constexpr(is_simd)
            this->pack = F32x4::sub(F32x4::zero(), this->pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] = -this->data[i];
        return *this;
    }

    constexpr Vect normalize() noexcept
        requires is_float
    {
        T l = norm();
        if constexpr(is_simd)
            this->pack = F32x4::div(this->pack, F32x4::set1(l));
        else
            for(u64 i = 0; i < N; i++) this->data[i] /= l;
        return *this;
    }

    constexpr Vect unit() const noexcept
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

    constexpr Vect<T, N - 1> proj() const noexcept
        requires(N > 2)
    {
        Vect<T, N - 1> r;
        for(u64 i = 0; i < N - 1; i++) r.data[i] = this->data[i] / this->data[N - 1];
        return r;
    }

    template<typename U>
    constexpr Vect<U, N> as() const noexcept {
        Vect<U, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = static_cast<U>(this->data[i]);
        return r;
    }

    constexpr T norm2() const noexcept {
        if constexpr(is_simd)
            return F32x4::dp(this->pack, this->pack);
        else {
            T r = T{};
            for(u64 i = 0; i < N; i++) r += this->data[i] * this->data[i];
            return r;
        }
    }

    constexpr T min() const noexcept {
        T r = this->data[0];
        for(u64 i = 1; i < N; i++) r = Math::min(r, this->data[i]);
        return r;
    }

    constexpr T max() const noexcept {
        T r = this->data[0];
        for(u64 i = 1; i < N; i++) r = Math::max(r, this->data[i]);
        return r;
    }

    constexpr Vect<T, N> floor() noexcept
        requires is_float
    {
        Vect<T, N> r;
        if constexpr(is_simd)
            r.pack = F32x4::floor(this->pack);
        else
            for(u64 i = 0; i < N; i++) r.data[i] = Math::floor(this->data[i]);
        return r;
    }

    constexpr Vect<T, N> ceil() noexcept
        requires is_float
    {
        Vect<T, N> r;
        if constexpr(is_simd)
            r.pack = F32x4::ceil(this->pack);
        else
            for(u64 i = 0; i < N; i++) r.data[i] = Math::ceil(this->data[i]);
        return r;
    }

    constexpr T norm() const noexcept
        requires is_float
    {
        return Math::sqrt(norm2());
    }

    constexpr const T* begin() const noexcept {
        return this->data;
    }
    constexpr const T* end() const noexcept {
        return this->data + N;
    }
    constexpr T* begin() noexcept {
        return this->data;
    }
    constexpr T* end() noexcept {
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

    Vect<T, 2>& xy() noexcept {
        return *reinterpret_cast<Vect<T, 2>*>(&x);
    }
    Vect<T, 2>& yz() noexcept {
        return *reinterpret_cast<Vect<T, 2>*>(&y);
    }
    const Vect<T, 2>& xy() const noexcept {
        return *reinterpret_cast<const Vect<T, 2>*>(&x);
    }
    const Vect<T, 2>& yz() const noexcept {
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

    Vect<T, 2>& xy() noexcept {
        return *reinterpret_cast<Vect<T, 2>*>(&x);
    }
    Vect<T, 2>& yz() noexcept {
        return *reinterpret_cast<Vect<T, 2>*>(&y);
    }
    Vect<T, 2>& zw() noexcept {
        return *reinterpret_cast<Vect<T, 2>*>(&z);
    }
    const Vect<T, 2>& xy() const noexcept {
        return *reinterpret_cast<const Vect<T, 2>*>(&x);
    }
    const Vect<T, 2>& yz() const noexcept {
        return *reinterpret_cast<const Vect<T, 2>*>(&y);
    }
    const Vect<T, 2>& zw() const noexcept {
        return *reinterpret_cast<const Vect<T, 2>*>(&z);
    }

    Vect<T, 3>& xyz() noexcept {
        return *reinterpret_cast<Vect<T, 3>*>(&x);
    }
    Vect<T, 3>& yzw() noexcept {
        return *reinterpret_cast<Vect<T, 3>*>(&y);
    }
    const Vect<T, 3>& xyz() const noexcept {
        return *reinterpret_cast<const Vect<T, 3>*>(&x);
    }
    const Vect<T, 3>& yzw() const noexcept {
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

    Vect<f32, 2>& xy() noexcept {
        return *reinterpret_cast<Vect<f32, 2>*>(&x);
    }
    Vect<f32, 2>& yz() noexcept {
        return *reinterpret_cast<Vect<f32, 2>*>(&y);
    }
    Vect<f32, 2>& zw() noexcept {
        return *reinterpret_cast<Vect<f32, 2>*>(&z);
    }
    const Vect<f32, 2>& xy() const noexcept {
        return *reinterpret_cast<const Vect<f32, 2>*>(&x);
    }
    const Vect<f32, 2>& yz() const noexcept {
        return *reinterpret_cast<const Vect<f32, 2>*>(&y);
    }
    const Vect<f32, 2>& zw() const noexcept {
        return *reinterpret_cast<const Vect<f32, 2>*>(&z);
    }

    Vect<f32, 3>& xyz() noexcept {
        return *reinterpret_cast<Vect<f32, 3>*>(&x);
    }
    Vect<f32, 3>& yzw() noexcept {
        return *reinterpret_cast<Vect<f32, 3>*>(&y);
    }
    const Vect<f32, 3>& xyz() const noexcept {
        return *reinterpret_cast<const Vect<f32, 3>*>(&x);
    }
    const Vect<f32, 3>& yzw() const noexcept {
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

constexpr Vec3 cross(Vec3 l, Vec3 r) noexcept {
    return Vec3{l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x};
}

template<typename T, u64 N>
constexpr Vect<T, N> min(Vect<T, N> x, Vect<T, N> y) noexcept {
    if constexpr(Vect<T, N>::is_simd)
        return Vect<T, N>{F32x4::min(x.pack, y.pack)};
    else {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = Math::min(x.data[i], y.data[i]);
        return r;
    }
}

template<typename T, u64 N>
constexpr Vect<T, N> max(Vect<T, N> x, Vect<T, N> y) noexcept {
    if constexpr(Vect<T, N>::is_simd)
        return Vect<T, N>{F32x4::max(x.pack, y.pack)};
    else {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = Math::max(x.data[i], y.data[i]);
        return r;
    }
}

template<typename T, u64 N>
constexpr Vect<T, N> abs(Vect<T, N> x) noexcept
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
constexpr T dot(Vect<T, N> x, Vect<T, N> y) noexcept {
    if constexpr(Vect<T, N>::is_simd)
        return F32x4::dp(x.pack, y.pack);
    else {
        T r = {};
        for(u64 i = 0; i < N; i++) r += x.data[i] * y.data[i];
        return r;
    }
}

template<Float T, u64 N>
constexpr Vect<T, N> lerp(Vect<T, N> min, Vect<T, N> max, T dist) noexcept {
    return min + (max - min) * dist;
}

template<typename T, u64 N>
constexpr Vect<T, N> clamp(Vect<T, N> x, Vect<T, N> min, Vect<T, N> max) noexcept {
    return Math::max(Math::min(x, max), min);
}

template<Float T, u64 N>
constexpr Vect<T, N> normalize(Vect<T, N> x) noexcept {
    return x.unit();
}

struct Mat4 {

    union {
        f32 data[16];
        F32x4 pack[4];
        Vec4 columns[4];
    };

    constexpr Mat4() noexcept
        : columns{Vec4{1.0f, 0.0f, 0.0f, 0.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f},
                  Vec4{0.0f, 0.0f, 1.0f, 0.0f}, Vec4{0.0f, 0.0f, 0.0f, 1.0f}} {
    }
    constexpr explicit Mat4(Vec4 x, Vec4 y, Vec4 z, Vec4 w) noexcept : columns{x, y, z, w} {
    }

    template<typename... Ts>
        requires Length<16, Ts...> && All_Are<f32, Ts...>
    constexpr explicit Mat4(Ts... args) noexcept : data{args...} {
    }

    constexpr ~Mat4() noexcept = default;

    constexpr Mat4(const Mat4&) noexcept = default;
    constexpr Mat4& operator=(const Mat4&) noexcept = default;
    constexpr Mat4(Mat4&&) noexcept = default;
    constexpr Mat4& operator=(Mat4&&) noexcept = default;

    static Mat4 look_at(Vec3 pos, Vec3 at, Vec3 up) noexcept;
    static Mat4 scale(Vec3 s) noexcept;
    static Mat4 rotate(f32 a, Vec3 axis) noexcept;
    static Mat4 translate(Vec3 v) noexcept;
    static Mat4 ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) noexcept;
    static Mat4 proj(f32 fov, f32 ar, f32 n) noexcept;
    static Mat4 rotate_y_to(Vec3 dir) noexcept;
    static Mat4 rotate_z_to(Vec3 dir) noexcept;
    static Mat4 inverse(Mat4 m) noexcept;
    static Mat4 transpose(Mat4 m) noexcept;

    bool operator==(Mat4 m) const noexcept;
    bool operator!=(Mat4 m) const noexcept;

    Mat4 operator+(Mat4 m) const noexcept;
    Mat4 operator-(Mat4 m) const noexcept;
    Mat4 operator*(Mat4 m) const noexcept;

    Mat4 operator+(f32 s) const noexcept;
    Mat4 operator-(f32 s) const noexcept;
    Mat4 operator*(f32 s) const noexcept;
    Mat4 operator/(f32 s) const noexcept;

    Vec4& operator[](u64 idx) noexcept;
    Vec4 operator[](u64 idx) const noexcept;

    Vec4 operator*(Vec4 v) const noexcept;
    Vec3 operator*(Vec3 v) const noexcept;
    Vec3 rotate(Vec3 v) const noexcept;

    Mat4 T() const noexcept;
    Mat4 inverse() const noexcept;
    Vec3 to_euler() const noexcept;

    constexpr const Vec4* begin() const noexcept {
        return columns;
    }
    constexpr const Vec4* end() const noexcept {
        return columns + 4;
    }
    constexpr Vec4* begin() noexcept {
        return columns;
    }
    constexpr Vec4* end() noexcept {
        return columns + 4;
    }

    static Mat4 I;
    static Mat4 zero;
    static Mat4 swap_x_z;
};

struct Quat : detail::Vect_Base<f32, 4> {

    using Base = detail::Vect_Base<f32, 4>;

    constexpr Quat() noexcept : Base{0.0f, 0.0f, 0.0f, 1.0f} {
    }
    constexpr explicit Quat(f32 x, f32 y, f32 z, f32 w) noexcept : Base{x, y, z, w} {
    }
    constexpr explicit Quat(Vec3 complex, f32 real) noexcept
        : Base{complex.x, complex.y, complex.z, real} {
    }
    constexpr explicit Quat(Vec4 src) noexcept : Base{src} {
    }

    constexpr ~Quat() noexcept = default;

    constexpr Quat(const Quat&) noexcept = default;
    constexpr Quat& operator=(const Quat&) noexcept = default;
    constexpr Quat(Quat&&) noexcept = default;
    constexpr Quat& operator=(Quat&&) noexcept = default;

    static Quat axis_angle(Vec3 axis, f32 angle) noexcept {
        axis.normalize();
        angle = Math::radians(angle) / 2.0f;
        f32 sin = Math::sin(angle);
        f32 x = sin * axis.x;
        f32 y = sin * axis.y;
        f32 z = sin * axis.z;
        f32 w = Math::cos(angle);
        return Quat(x, y, z, w).unit();
    }
    static Quat euler(Vec3 angles) noexcept {
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

    constexpr f32& operator[](int idx) noexcept {
        return data[idx];
    }
    constexpr f32 operator[](int idx) const noexcept {
        return data[idx];
    }

    constexpr Quat conjugate() const noexcept {
        return Quat(-x, -y, -z, w);
    }
    Quat inverse() const noexcept {
        return conjugate().unit();
    }
    constexpr Vec3 complex() const noexcept {
        return Vec3(x, y, z);
    }
    constexpr f32 real() const noexcept {
        return w;
    }

    constexpr f32 norm2() const noexcept {
        return x * x + y * y + z * z + w * w;
    }
    f32 norm() const noexcept {
        return Math::sqrt(norm2());
    }
    Quat unit() const noexcept {
        f32 n = norm();
        return Quat(x / n, y / n, z / n, w / n);
    }

    constexpr Quat operator*(const Quat& r) const noexcept {
        return Quat(y * r.z - z * r.y + x * r.w + w * r.x, z * r.x - x * r.z + y * r.w + w * r.y,
                    x * r.y - y * r.x + z * r.w + w * r.z, w * r.w - x * r.x - y * r.y - z * r.z);
    }
    constexpr Quat operator*(f32 s) const noexcept {
        return Quat(s * x, s * y, s * z, s * w);
    }

    constexpr Quat operator+(const Quat& r) const noexcept {
        return Quat(x + r.x, y + r.y, z + r.z, w + r.w);
    }
    constexpr Quat operator-(const Quat& r) const noexcept {
        return Quat(x - r.x, y - r.y, z - r.z, w - r.w);
    }
    constexpr Quat operator-() const noexcept {
        return Quat(-x, -y, -z, -w);
    }

    Vec3 to_euler() const noexcept {
        return unit().to_mat().to_euler();
    }

    constexpr Mat4 to_mat() const noexcept {
        return Mat4{
            Vec4{1 - 2 * y * y - 2 * z * z, 2 * x * y + 2 * z * w, 2 * x * z - 2 * y * w, 0.0f},
            Vec4{2 * x * y - 2 * z * w, 1 - 2 * x * x - 2 * z * z, 2 * y * z + 2 * x * w, 0.0f},
            Vec4{2 * x * z + 2 * y * w, 2 * y * z - 2 * x * w, 1 - 2 * x * x - 2 * y * y, 0.0f},
            Vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    }

    constexpr Vec3 rotate(Vec3 v) const noexcept {
        return (((*this) * Quat(v, 0)) * conjugate()).complex();
    }

    constexpr bool operator==(const Quat& v) const noexcept {
        return x == v.x && y == v.y && z == v.z && w == v.w;
    }
    constexpr bool operator!=(const Quat& v) const noexcept {
        return x != v.x || y != v.y || z != v.z || w != v.w;
    }

    constexpr const f32* begin() const noexcept {
        return this->data;
    }
    constexpr const f32* end() const noexcept {
        return this->data + 4;
    }
    constexpr f32* begin() noexcept {
        return this->data;
    }
    constexpr f32* end() noexcept {
        return this->data + 4;
    }
};

struct BBox {

    constexpr BBox() noexcept : min(Limits<f32>::max()), max(Limits<f32>::min()) {
    }
    constexpr explicit BBox(Vec3 min, Vec3 max) noexcept : min(min), max(max) {
    }

    constexpr ~BBox() noexcept = default;

    constexpr BBox(const BBox&) noexcept = default;
    constexpr BBox& operator=(const BBox&) noexcept = default;
    constexpr BBox(BBox&&) noexcept = default;
    constexpr BBox& operator=(BBox&&) noexcept = default;

    constexpr void reset() noexcept {
        min = Vec3(Limits<f32>::max());
        max = Vec3(Limits<f32>::min());
    }

    constexpr void enclose(Vec3 point) noexcept {
        min = Math::min(min, point);
        max = Math::max(max, point);
    }
    constexpr void enclose(BBox box) noexcept {
        min = Math::min(min, box.min);
        max = Math::max(max, box.max);
    }

    constexpr Vec3 center() const noexcept {
        return (min + max) * 0.5f;
    }

    constexpr bool empty() const noexcept {
        return min.x > max.x || min.y > max.y || min.z > max.z;
    }

    constexpr f32 surface_area() const noexcept {
        if(empty()) return 0.0f;
        Vec3 extent = max - min;
        return 2.0f * (extent.x * extent.z + extent.x * extent.y + extent.y * extent.z);
    }

    constexpr void transform(const Mat4& T) noexcept {
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

    constexpr void project(const Mat4& proj, Vec2& min_out, Vec2& max_out) const noexcept {

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
constexpr Math::Vect<T, N> operator+(T s, Math::Vect<T, N> v) noexcept {
    if constexpr(Math::Vect<T, N>::is_simd)
        return Math::Vect<T, N>{SIMD::F32x4::add(v.pack, SIMD::F32x4::set1(s))};
    else {
        Math::Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = v.data[i] + s;
        return r;
    }
}

template<typename T, u64 N>
constexpr Math::Vect<T, N> operator-(T s, Math::Vect<T, N> v) noexcept {
    if constexpr(Math::Vect<T, N>::is_simd)
        return Math::Vect<T, N>{SIMD::F32x4::sub(v.pack, SIMD::F32x4::set1(s))};
    else {
        Math::Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = s - v.data[i];
        return r;
    }
}

template<typename T, u64 N>
constexpr Math::Vect<T, N> operator*(T s, Math::Vect<T, N> v) noexcept {
    if constexpr(Math::Vect<T, N>::is_simd)
        return Math::Vect<T, N>{SIMD::F32x4::mul(v.pack, SIMD::F32x4::set1(s))};
    else {
        Math::Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = v.data[i] * s;
        return r;
    }
}

template<typename T, u64 N>
constexpr Math::Vect<T, N> operator/(T s, Math::Vect<T, N> v) noexcept {
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
    constexpr static u64 hash(const Math::Vect<T, N>& v) noexcept {
        u64 h = 0;
        for(u64 i = 0; i < N; i++) h = hash_combine(h, rpp::hash(v[i]));
        return h;
    }
};

} // namespace Hash

namespace Format {

template<Float F, u64 N>
struct Measure<Math::Vect<F, N>> {
    constexpr static u64 measure(const Math::Vect<F, N>& vect) noexcept {
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
    constexpr static u64 measure(const Math::Vect<I, N>& vect) noexcept {
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
    constexpr static u64 measure(const Math::Mat4& mat) noexcept {
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
    constexpr static u64 measure(const Math::Quat& quat) noexcept {
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
    constexpr static u64 measure(const Math::BBox& bbox) noexcept {
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
    static u64 write(String<O>& output, u64 idx, const Math::Vect<F, N>& vect) noexcept {
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
    static u64 write(String<O>& output, u64 idx, const Math::Vect<I, N>& vect) noexcept {
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
    static u64 write(String<O>& output, u64 idx, const Math::Vect<I, N>& vect) noexcept {
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
    static u64 write(String<O>& output, u64 idx, const Math::Mat4& mat) noexcept {
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
    static u64 write(String<O>& output, u64 idx, const Math::Quat& quat) noexcept {
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
    static u64 write(String<O>& output, u64 idx, const Math::BBox& bbox) noexcept {
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
