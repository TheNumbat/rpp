
#pragma once

namespace rpp {
namespace Math {

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
    typename If<N == 2, Vec2_base<T>,
                typename If<N == 3, Vec3_base<T>,
                            typename If<N == 4, Vec4_base<T>, VecN_base<T, N>>::type>::type>::type;

template<typename T, u64 N>
struct Vect : Vect_Base<T, N> {

    using Base = Vect_Base<T, N>;
    static constexpr bool is_float = Float<T>;
    static constexpr bool is_simd = is_float && N == 4;

    constexpr Vect() : Base{} {
        if constexpr(is_simd)
            this->pack = _mm_setzero_ps();
        else
            for(u64 i = 0; i < N; i++) this->data[i] = T{};
    };

    template<typename S = T>
        requires is_simd && Same<S, T>
    constexpr explicit Vect(__m128 p) : Base{} {
        this->pack = p;
    }

    constexpr explicit Vect(T x) : Base{} {
        for(u64 i = 0; i < N; i++) this->data[i] = x;
    }

    constexpr explicit Vect(Vect<T, N - 1> f, T s) : Base{} {
        for(u64 i = 0; i < N - 1; i++) this->data[i] = f[i];
        this->data[N - 1] = s;
    }

    template<typename S, typename... Ss>
        requires All<T, S, Ss...> && Length<N - 1, Ss...>
    constexpr explicit Vect(S first, Ss... rest) : Base{first, rest...} {
    }

    T& operator[](u64 idx) {
        return this->data[idx];
    }
    const T& operator[](u64 idx) const {
        return this->data[idx];
    }

    Vect operator+=(Vect v) {
        if constexpr(is_simd)
            this->pack = _mm_add_ps(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] += v.data[i];
        return *this;
    }
    Vect operator-=(Vect v) {
        if constexpr(is_simd)
            this->pack = _mm_sub_ps(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] -= v.data[i];
        return *this;
    }
    Vect operator*=(Vect v) {
        if constexpr(is_simd)
            this->pack = _mm_mul_ps(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] *= v.data[i];
        return *this;
    }
    Vect operator/=(Vect v) {
        if constexpr(is_simd)
            this->pack = _mm_div_ps(this->pack, v.pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] /= v.data[i];
        return *this;
    }

    Vect operator+=(T s) {
        if constexpr(is_simd)
            this->pack = _mm_add_ps(this->pack, _mm_set1_ps(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] += s;
        return *this;
    }
    Vect operator-=(T s) {
        if constexpr(is_simd)
            this->pack = _mm_sub_ps(this->pack, _mm_set1_ps(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] -= s;
        return *this;
    }
    Vect operator*=(T s) {
        if constexpr(is_simd)
            this->pack = _mm_mul_ps(this->pack, _mm_set1_ps(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] *= s;
        return *this;
    }
    Vect operator/=(T s) {
        if constexpr(is_simd)
            this->pack = _mm_div_ps(this->pack, _mm_set1_ps(s));
        else
            for(u64 i = 0; i < N; i++) this->data[i] /= s;
        return *this;
    }

    Vect operator+(Vect o) const {
        if constexpr(is_simd)
            return {_mm_add_ps(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] + o.data[i];
            return r;
        }
    }
    Vect operator-(Vect o) const {
        if constexpr(is_simd)
            return {_mm_sub_ps(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] - o.data[i];
            return r;
        }
    }
    Vect operator*(Vect o) const {
        if constexpr(is_simd)
            return {_mm_mul_ps(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] * o.data[i];
            return r;
        }
    }
    Vect operator/(Vect o) const {
        if constexpr(is_simd)
            return {_mm_div_ps(this->pack, o.pack)};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] / o.data[i];
            return r;
        }
    }

    Vect operator+(T s) const {
        if constexpr(is_simd)
            return {_mm_add_ps(this->pack, _mm_set1_ps(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] + s;
            return r;
        }
    }
    Vect operator-(T s) const {
        if constexpr(is_simd)
            return {_mm_sub_ps(this->pack, _mm_set1_ps(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] - s;
            return r;
        }
    }
    Vect operator*(T s) const {
        if constexpr(is_simd)
            return {_mm_mul_ps(this->pack, _mm_set1_ps(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] * s;
            return r;
        }
    }
    Vect operator/(T s) const {
        if constexpr(is_simd)
            return {_mm_div_ps(this->pack, _mm_set1_ps(s))};
        else {
            Vect r;
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] / s;
            return r;
        }
    }

    bool operator==(Vect o) const {
        if constexpr(is_simd)
            return _mm_movemask_ps(_mm_cmpeq_ps(this->pack, o.pack)) == 0xf;
        else {
            for(u64 i = 0; i < N; i++)
                if(this->data[i] != o.data[i]) return false;
            return true;
        }
    }
    bool operator!=(Vect o) const {
        if constexpr(is_simd)
            return _mm_movemask_ps(_mm_cmpeq_ps(this->pack, o.pack)) != 0xf;
        else {
            for(u64 i = 0; i < N; i++)
                if(this->data[i] != o.data[i]) return true;
            return false;
        }
    }

    Vect abs() const {
        Vect r;
        for(u64 i = 0; i < N; i++) r.data[i] = Math::abs(this->data[i]);
        return r;
    }
    Vect operator-() {
        if constexpr(is_simd)
            this->pack = _mm_sub_ps(_mm_set1_ps(0.0f), this->pack);
        else
            for(u64 i = 0; i < N; i++) this->data[i] = -this->data[i];
        return *this;
    }

    template<typename S = T>
        requires is_float && Same<S, T>
    Vect normalize() {
        T l = norm();
        if constexpr(is_simd)
            this->pack = _mm_div_ps(this->pack, _mm_set1_ps(l));
        else
            for(u64 i = 0; i < N; i++) this->data[i] /= l;
        return *this;
    }

    template<typename S = T>
        requires is_float && Same<S, T>
    Vect unit() const {
        Vect r;
        T l = norm();
        if constexpr(is_simd)
            r.pack = _mm_div_ps(this->pack, _mm_set1_ps(l));
        else
            for(u64 i = 0; i < N; i++) r.data[i] = this->data[i] / l;
        return r;
    }

    Vect<T, N - 1> proj() const {
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
        return dot(*this, *this);
    }

    T min() const {
        T r = this->data[0];
        for(u64 i = 1; i < N; i++) r = _MIN(r, this->data[i]);
        return r;
    }

    T max() const {
        T r = this->data[0];
        for(u64 i = 1; i < N; i++) r = _MAX(r, this->data[i]);
        return r;
    }

    template<typename S = T>
        requires is_float && Same<S, T>
    Vect<T, N> floor() {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = Math::floor(this->data[i]);
        return r;
    }

    template<typename S = T>
        requires is_float && Same<S, T>
    Vect<T, N> ceil() {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = Math::ceil(this->data[i]);
        return r;
    }

    template<typename S = T>
        requires is_float && Same<S, T>
    T norm() const {
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
        struct {
            Vect<T, 2> xy;
            T _z;
        };
        struct {
            T _x;
            Vect<T, 2> yz;
        };
        T data[3];
    };
};
template<typename T>
struct Vec4_base {
    union {
        struct {
            T x, y, z, w;
        };
        struct {
            Vect<T, 2> xy;
            Vect<T, 2> zw;
        };
        struct {
            T _x;
            Vect<T, 2> yz;
            T _w;
        };
        struct {
            Vect<T, 3> xyz;
            T __w;
        };
        struct {
            T __x;
            Vect<T, 3> yzw;
        };
        T data[4];
    };
};
template<>
struct Vec4_base<f32> {
    union {
        struct {
            f32 x, y, z, w;
        };
        struct {
            Vect<f32, 2> xy;
            Vect<f32, 2> zw;
        };
        struct {
            f32 _x;
            Vect<f32, 2> yz;
            f32 _w;
        };
        struct {
            Vect<f32, 3> xyz;
            f32 __w;
        };
        struct {
            f32 __x;
            Vect<f32, 3> yzw;
        };
        f32 data[4];
        __m128 pack;
    };
};
template<typename T, u64 N>
struct VecN_base {
    T data[N];
};

template<typename T, u64 N>
Vect<T, N> operator+(float s, Vect<T, N> v) {
    if constexpr(Vect<T, N>::is_simd)
        return {_mm_add_ps(v.pack, _mm_set1_ps(s))};
    else {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = v.data[i] + s;
        return r;
    }
}

template<typename T, u64 N>
Vect<T, N> operator-(float s, Vect<T, N> v) {
    if constexpr(Vect<T, N>::is_simd)
        return {_mm_sub_ps(v.pack, _mm_set1_ps(s))};
    else {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = s - v.data[i];
        return r;
    }
}

template<typename T, u64 N>
Vect<T, N> operator*(T s, Vect<T, N> v) {
    if constexpr(Vect<T, N>::is_simd)
        return {_mm_mul_ps(v.pack, _mm_set1_ps(s))};
    else {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = v.data[i] * s;
        return r;
    }
}

template<typename T, u64 N>
Vect<T, N> operator/(T s, Vect<T, N> v) {
    if constexpr(Vect<T, N>::is_simd)
        return {_mm_div_ps(v.pack, _mm_set1_ps(s))};
    else {
        Vect<T, N> r;
        for(u64 i = 0; i < N; i++) r.data[i] = s / v.data[i];
        return r;
    }
}

template<typename T, u64 N>
Vect<T, N> min(Vect<T, N> x, Vect<T, N> y) {
    if constexpr(Vect<T, N>::is_simd) return {_mm_min_ps(x.pack, y.pack)};
    Vect<T, N> r;
    for(u64 i = 0; i < N; i++) r.data[i] = Math::min(x.data[i], y.data[i]);
    return r;
}

template<typename T, u64 N>
Vect<T, N> max(Vect<T, N> x, Vect<T, N> y) {
    if constexpr(Vect<T, N>::is_simd) return {_mm_max_ps(x.pack, y.pack)};
    Vect<T, N> r;
    for(u64 i = 0; i < N; i++) r.data[i] = Math::max(x.data[i], y.data[i]);
    return r;
}

template<typename T, u64 N>
Vect<T, N> abs(Vect<T, N> x) {
    if constexpr(Vect<T, N>::is_simd) return {_mm_andnot_ps(_mm_set1_ps(-0.0f), x.pack)};
    Vect<T, N> r;
    for(u64 i = 0; i < N; i++) r.data[i] = Math::abs(x.data[i]);
    return r;
}

template<typename T, u64 N>
T dot(Vect<T, N> x, Vect<T, N> y) {
    if constexpr(Vect<T, N>::is_simd) return Vect<f32, 4>{_mm_dp_ps(x.pack, y.pack, 0xf1)}.x;
    T r = {};
    for(u64 i = 0; i < N; i++) r += x.data[i] * y.data[i];
    return r;
}

template<typename T, u64 N>
Vect<T, N> lerp(Vect<T, N> min, Vect<T, N> max, T dist) {
    return min + (max - min) * dist;
}

template<typename T, u64 N>
Vect<T, N> clamp(Vect<T, N> x, Vect<T, N> min, Vect<T, N> max) {
    return max(min(x, max), min);
}

template<typename T, u64 N>
Vect<T, N> normalize(Vect<T, N> x) {
    return x.unit();
}

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

struct Mat4;
inline Mat4 inverse(Mat4 m);
inline Mat4 transpose(Mat4 m);

struct Mat4 {

    union {
        f32 data[16];
        __m128 pack[4];
        Vec4 columns[4];
    };

    Mat4()
        : columns{Vec4{1.0f, 0.0f, 0.0f, 0.0f}, Vec4{0.0f, 1.0f, 0.0f, 0.0f},
                  Vec4{0.0f, 0.0f, 1.0f, 0.0f}, Vec4{0.0f, 0.0f, 0.0f, 1.0f}} {
    }
    explicit Mat4(Vec4 x, Vec4 y, Vec4 z, Vec4 w) : columns{x, y, z, w} {
    }

    template<typename... Ts>
        requires Length<16, Ts...> && All<f32, Ts...>
    explicit Mat4(Ts... args) : data{args...} {
    }

    static Mat4 look_at(Vec3 pos, Vec3 at, Vec3 up);
    static Mat4 scale(Vec3 s);
    static Mat4 rotate(f32 a, Vec3 axis);
    static Mat4 translate(Vec3 v);
    static Mat4 ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
    static Mat4 proj(f32 fov, f32 ar, f32 n);
    static Mat4 transpose(Mat4 m);
    static Mat4 rotate_to(Vec3 dir);
    static Mat4 rotate_z_to(Vec3 dir);

    bool operator==(Mat4 v) {
        return columns[0] == v.columns[0] && columns[1] == v.columns[1] &&
               columns[2] == v.columns[2] && columns[3] == v.columns[3];
    }
    bool operator!=(Mat4 v) {
        return columns[0] != v.columns[0] || columns[1] != v.columns[1] ||
               columns[2] != v.columns[2] || columns[3] != v.columns[3];
    }

    Mat4 operator+=(Mat4 v) {
        for(u64 i = 0; i < 4; i++) pack[i] = _mm_add_ps(pack[i], v.pack[i]);
        return *this;
    }
    Mat4 operator-=(Mat4 v) {
        for(u64 i = 0; i < 4; i++) pack[i] = _mm_sub_ps(pack[i], v.pack[i]);
        return *this;
    }

    Mat4 operator+=(f32 s) {
        __m128 add = _mm_set1_ps(s);
        for(u64 i = 0; i < 4; i++) pack[i] = _mm_add_ps(pack[i], add);
        return *this;
    }
    Mat4 operator-=(f32 s) {
        __m128 sub = _mm_set1_ps(s);
        for(u64 i = 0; i < 4; i++) pack[i] = _mm_sub_ps(pack[i], sub);
        return *this;
    }
    Mat4 operator*=(f32 s) {
        __m128 mul = _mm_set1_ps(s);
        for(u64 i = 0; i < 4; i++) pack[i] = _mm_mul_ps(pack[i], mul);
        return *this;
    }
    Mat4 operator/=(f32 s) {
        __m128 div = _mm_set1_ps(s);
        for(u64 i = 0; i < 4; i++) pack[i] = _mm_div_ps(pack[i], div);
        return *this;
    }

    Vec4& operator[](u64 idx) {
        return columns[idx];
    }
    Vec4 operator[](u64 idx) const {
        return columns[idx];
    }

    Mat4 operator+(Mat4 m) const {
        Mat4 ret;
        for(u64 i = 0; i < 4; i++) ret.pack[i] = _mm_add_ps(pack[i], m.pack[i]);
        return ret;
    }
    Mat4 operator-(Mat4 m) const {
        Mat4 ret;
        for(u64 i = 0; i < 4; i++) ret.pack[i] = _mm_sub_ps(pack[i], m.pack[i]);
        return ret;
    }

    Mat4 operator+(f32 s) const {
        Mat4 ret;
        __m128 add = _mm_set1_ps(s);
        for(u64 i = 0; i < 4; i++) ret.pack[i] = _mm_add_ps(pack[i], add);
        return ret;
    }
    Mat4 operator-(f32 s) const {
        Mat4 ret;
        __m128 sub = _mm_set1_ps(s);
        for(u64 i = 0; i < 4; i++) ret.pack[i] = _mm_sub_ps(pack[i], sub);
        return ret;
    }
    Mat4 operator*(f32 s) const {
        Mat4 ret;
        __m128 mul = _mm_set1_ps(s);
        for(u64 i = 0; i < 4; i++) ret.pack[i] = _mm_mul_ps(pack[i], mul);
        return ret;
    }
    Mat4 operator/(f32 s) const {
        Mat4 ret;
        __m128 div = _mm_set1_ps(s);
        for(u64 i = 0; i < 4; i++) ret.pack[i] = _mm_div_ps(pack[i], div);
        return ret;
    }

    Mat4 operator*=(Mat4 v) {
        *this = *this * v;
        return *this;
    }
    Mat4 operator*(Mat4 m) const {
        Mat4 ret;
        for(u64 i = 0; i < 4; i++) {
            ret.pack[i] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_set1_ps(m[i][0]), pack[0]),
                                                _mm_mul_ps(_mm_set1_ps(m[i][1]), pack[1])),
                                     _mm_add_ps(_mm_mul_ps(_mm_set1_ps(m[i][2]), pack[2]),
                                                _mm_mul_ps(_mm_set1_ps(m[i][3]), pack[3])));
        }
        return ret;
    }

    Vec4 operator*(Vec4 v) const {
        return Vec4{_mm_add_ps(_mm_add_ps(_mm_mul_ps(pack[0], _mm_set1_ps(v.x)),
                                          _mm_mul_ps(pack[1], _mm_set1_ps(v.y))),
                               _mm_add_ps(_mm_mul_ps(pack[2], _mm_set1_ps(v.z)),
                                          _mm_mul_ps(pack[3], _mm_set1_ps(v.w))))};
    }

    Vec3 operator*(Vec3 v) const {
        Vec4 r = *this * Vec4{v, 1.0f};
        return r.proj();
    }
    Vec3 rotate(Vec3 v) const {
        Vec4 r = *this * Vec4{v, 0.0f};
        return r.xyz;
    }

    Mat4 T() const {
        return transpose(*this);
    }
    Mat4 inverse() const {
        return Math::inverse(*this);
    }

    const Vec4* begin() const {
        return columns;
    }
    const Vec4* end() const {
        return columns + 4;
    }
    Vec4* begin() {
        return columns;
    }
    Vec4* end() {
        return columns + 4;
    }

    Vec3 to_euler() const {

        bool single = true;
        static const f32 singularity[] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
                                          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0};
        for(int i = 0; i < 12 && single; i++) {
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

    static Mat4 I, zero, swap_x_z;
};

inline Mat4 Mat4::I = Mat4{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
inline Mat4 Mat4::zero = Mat4{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
inline Mat4 Mat4::swap_x_z = Mat4{0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

#define MakeShuffleMask(x, y, z, w) (x | (y << 2) | (z << 4) | (w << 6))
#define VecSwizzleMask(vec, mask) _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w) VecSwizzleMask(vec, MakeShuffleMask(x, y, z, w))
#define VecSwizzle1(vec, x) VecSwizzleMask(vec, MakeShuffleMask(x, x, x, x))
#define VecSwizzle_0022(vec) _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec) _mm_movehdup_ps(vec)
#define VecShuffle(vec1, vec2, x, y, z, w) _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x, y, z, w))
#define VecShuffle_0101(vec1, vec2) _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2) _mm_movehl_ps(vec2, vec1)
inline __m128 Mat2Mul(__m128 vec1, __m128 vec2) {
    return _mm_add_ps(_mm_mul_ps(vec1, VecSwizzle(vec2, 0, 3, 0, 3)),
                      _mm_mul_ps(VecSwizzle(vec1, 1, 0, 3, 2), VecSwizzle(vec2, 2, 1, 2, 1)));
}
inline __m128 Mat2AdjMul(__m128 vec1, __m128 vec2) {
    return _mm_sub_ps(_mm_mul_ps(VecSwizzle(vec1, 3, 3, 0, 0), vec2),
                      _mm_mul_ps(VecSwizzle(vec1, 1, 1, 2, 2), VecSwizzle(vec2, 2, 3, 0, 1)));
}
inline __m128 Mat2MulAdj(__m128 vec1, __m128 vec2) {
    return _mm_sub_ps(_mm_mul_ps(vec1, VecSwizzle(vec2, 3, 0, 3, 0)),
                      _mm_mul_ps(VecSwizzle(vec1, 1, 0, 3, 2), VecSwizzle(vec2, 2, 1, 2, 1)));
}
inline Mat4 inverse(Mat4 m) {
    __m128 A = VecShuffle_0101(m.pack[0], m.pack[1]);
    __m128 B = VecShuffle_2323(m.pack[0], m.pack[1]);
    __m128 C = VecShuffle_0101(m.pack[2], m.pack[3]);
    __m128 D = VecShuffle_2323(m.pack[2], m.pack[3]);

    __m128 detSub = _mm_sub_ps(_mm_mul_ps(VecShuffle(m.pack[0], m.pack[2], 0, 2, 0, 2),
                                          VecShuffle(m.pack[1], m.pack[3], 1, 3, 1, 3)),
                               _mm_mul_ps(VecShuffle(m.pack[0], m.pack[2], 1, 3, 1, 3),
                                          VecShuffle(m.pack[1], m.pack[3], 0, 2, 0, 2)));
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
    r.pack[0] = VecShuffle(X_, Y_, 3, 1, 3, 1);
    r.pack[1] = VecShuffle(X_, Y_, 2, 0, 2, 0);
    r.pack[2] = VecShuffle(Z_, W_, 3, 1, 3, 1);
    r.pack[3] = VecShuffle(Z_, W_, 2, 0, 2, 0);
    return r;
}

inline Mat4 Mat4::transpose(Mat4 m) {
    Mat4 ret;
    for(u64 i = 0; i < 4; i++)
        for(u64 j = 0; j < 4; j++) ret[i][j] = m[j][i];
    return ret;
}

inline Mat4 Mat4::ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    Mat4 ret;
    ret[0][0] = 2.0f / (r - l);
    ret[1][1] = 2.0f / (t - b);
    ret[2][2] = 2.0f / (n - f);
    ret[3][0] = (-l - r) / (r - l);
    ret[3][1] = (-b - t) / (t - b);
    ret[3][2] = -n / (f - n);
    return ret;
}

inline Mat4 Mat4::proj(f32 fov, f32 ar, f32 n) {
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

inline Mat4 Mat4::translate(Vec3 v) {
    Mat4 ret;
    ret[3].xyz = v;
    return ret;
}

inline Mat4 Mat4::rotate(f32 a, Vec3 axis) {
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

inline Mat4 Mat4::scale(Vec3 s) {
    Mat4 ret;
    ret[0][0] = s.x;
    ret[1][1] = s.y;
    ret[2][2] = s.z;
    return ret;
}

inline Mat4 Mat4::look_at(Vec3 pos, Vec3 at, Vec3 up) {
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

inline Mat4 Mat4::rotate_to(Vec3 dir) {

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

inline Mat4 Mat4::rotate_z_to(Vec3 dir) {
    Mat4 y = rotate_to(dir);
    Vec4 _y = y[1];
    Vec4 _z = y[2];
    y[1] = _z;
    y[2] = -_y;
    return y;
}

struct Quat : Vect_Base<f32, 4> {

    using Base = Vect_Base<f32, 4>;
    Quat() : Base{0.0f, 0.0f, 0.0f, 1.0f} {
    }
    Quat(f32 x, f32 y, f32 z, f32 w) : Base{x, y, z, w} {
    }
    Quat(Vec3 complex, f32 real) : Base{complex.x, complex.y, complex.z, real} {
    }
    Quat(Vec4 src) : Base{src} {
    }

    static Quat axis_angle(Vec3 axis, f32 angle);
    static Quat euler(Vec3 angles);

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

inline Quat Quat::euler(Vec3 angles) {
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

inline Quat Quat::axis_angle(Vec3 axis, f32 angle) {
    axis.normalize();
    angle = Math::radians(angle) / 2.0f;
    f32 sin = Math::sin(angle);
    f32 x = sin * axis.x;
    f32 y = sin * axis.y;
    f32 z = sin * axis.z;
    f32 w = Math::cos(angle);
    return Quat(x, y, z, w).unit();
}

struct BBox {

    BBox() : min(Limits<f32>::max()), max(Limits<f32>::min()) {
    }
    BBox(Vec3 min, Vec3 max) : min(min), max(max) {
    }

    BBox(const BBox&) = default;
    BBox& operator=(const BBox&) = default;
    ~BBox() = default;

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
        min = max = T[3].xyz;
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

    void screen_bb(const Mat4& transform, Vec2& min_out, Vec2& max_out) const {

        min_out = Vec2(Limits<f32>::max());
        max_out = Vec2(Limits<f32>::min());

        Vec3 c[] = {Vec3(min.x, min.y, min.z), Vec3(max.x, min.y, min.z), Vec3(min.x, max.y, min.z),
                    Vec3(min.x, min.y, max.z), Vec3(max.x, max.y, min.z), Vec3(min.x, max.y, max.z),
                    Vec3(max.x, min.y, max.z), Vec3(max.x, max.y, max.z)};

        bool partially_behind = false, all_behind = true;
        for(auto& v : c) {
            Vec3 p = transform * v;
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
using Math::Vec2i;
using Math::Vec2u;
using Math::Vec3;
using Math::Vec3i;
using Math::Vec3u;
using Math::Vec4;
using Math::Vec4i;
using Math::Vec4u;
template<u64 N>
using VecN = Math::VecN<N>;
template<u64 N>
using VecNi = Math::VecNi<N>;
template<u64 N>
using VecNu = Math::VecNu<N>;

using Math::BBox;
using Math::Mat4;
using Math::Quat;

template<>
struct Reflect<Vec2> {
    using T = Vec2;
    static constexpr Literal name = "Vec2";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(x), FIELD(y)>;
};

template<>
struct Reflect<Vec3> {
    using T = Vec3;
    static constexpr Literal name = "Vec3";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(x), FIELD(y), FIELD(z)>;
};

template<>
struct Reflect<Vec4> {
    using T = Vec4;
    static constexpr Literal name = "Vec4";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(x), FIELD(y), FIELD(z), FIELD(w)>;
};

template<>
struct Reflect<Vec2i> {
    using T = Vec2i;
    static constexpr Literal name = "Vec2i";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(x), FIELD(y)>;
};

template<>
struct Reflect<Vec3i> {
    using T = Vec3i;
    static constexpr Literal name = "Vec3i";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(x), FIELD(y), FIELD(z)>;
};

template<>
struct Reflect<Vec4i> {
    using T = Vec4i;
    static constexpr Literal name = "Vec4i";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(x), FIELD(y), FIELD(z), FIELD(w)>;
};

template<>
struct Reflect<Vec2u> {
    using T = Vec2u;
    static constexpr Literal name = "Vec2u";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(x), FIELD(y)>;
};

template<>
struct Reflect<Vec3u> {
    using T = Vec3u;
    static constexpr Literal name = "Vec3u";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(x), FIELD(y), FIELD(z)>;
};

template<>
struct Reflect<Vec4u> {
    using T = Vec4u;
    static constexpr Literal name = "Vec4u";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(x), FIELD(y), FIELD(z), FIELD(w)>;
};

template<u64 N>
struct Reflect<VecN<N>> {
    using T = VecN<N>;
    static constexpr Literal name = "VecN";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data)>;
};

template<u64 N>
struct Reflect<VecNi<N>> {
    using T = VecNi<N>;
    static constexpr Literal name = "VecNi";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data)>;
};

template<u64 N>
struct Reflect<VecNu<N>> {
    using T = VecNu<N>;
    static constexpr Literal name = "VecNu";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data)>;
};

template<>
struct Reflect<Mat4> {
    using T = Mat4;
    static constexpr Literal name = "Mat4";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(columns)>;
};

template<>
struct Reflect<Quat> {
    using T = Quat;
    static constexpr Literal name = "Quat";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data)>;
};

template<>
struct Reflect<BBox> {
    using T = BBox;
    static constexpr Literal name = "BBox";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(min), FIELD(max)>;
};

} // namespace rpp
