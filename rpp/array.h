
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T, u64 N>
    requires(N > 0)
struct Array {
    constexpr static u64 capacity = N;

    constexpr Array() noexcept
        requires Default_Constructable<T>
    = default;

    template<typename... S>
        requires Length<N, S...> && All_Are<T, S...>
    constexpr explicit Array(S&&... init) noexcept : data_{rpp::forward<S>(init)...} {
    }

    template<typename... S>
        requires Length<N, S...> && All_Are<T, S...> && Copy_Constructable<T>
    constexpr explicit Array(const S&... init) noexcept : data_{T{init}...} {
    }

    constexpr ~Array() noexcept = default;

    constexpr Array(const Array& src) noexcept
        requires Copy_Constructable<T>
    = default;
    constexpr Array& operator=(const Array& src) noexcept
        requires Copy_Constructable<T>
    = default;

    constexpr Array(Array&& src) noexcept = default;
    constexpr Array& operator=(Array&& src) noexcept = default;

    [[nodiscard]] constexpr Array clone() const noexcept
        requires(Clone<T> || Copy_Constructable<T>) && Default_Constructable<T>
    {
        Array result;
        if constexpr(Trivially_Copyable<T>) {
            Libc::memcpy(result.data_, data_, sizeof(T) * N);
        } else if constexpr(Clone<T>) {
            for(u64 i = 0; i < N; i++) {
                result[i] = data_[i].clone();
            }
        } else {
            static_assert(Copy_Constructable<T>);
            for(u64 i = 0; i < N; i++) {
                result[i] = T{data_[i]};
            }
        }
        return result;
    }

    [[nodiscard]] constexpr T& operator[](u64 idx) noexcept {
        assert(idx < N);
        return data_[idx];
    }
    [[nodiscard]] constexpr const T& operator[](u64 idx) const noexcept {
        assert(idx < N);
        return data_[idx];
    }

    [[nodiscard]] constexpr T* data() {
        return data_;
    }
    [[nodiscard]] constexpr const T* data() const noexcept {
        return data_;
    }

    [[nodiscard]] constexpr u64 length() const noexcept {
        return capacity;
    }

    [[nodiscard]] constexpr const T* begin() const noexcept {
        return data_;
    }
    [[nodiscard]] constexpr const T* end() const noexcept {
        return data_ + N;
    }
    [[nodiscard]] constexpr T* begin() noexcept {
        return data_;
    }
    [[nodiscard]] constexpr T* end() noexcept {
        return data_ + N;
    }

    [[nodiscard]] Slice<T> slice() const noexcept {
        return Slice<T>{data_, N};
    }

private:
    T data_[N] = {};
    friend struct Reflect::Refl<Array>;
};

namespace Format {

template<Reflectable T, u64 N>
struct Measure<Array<T, N>> {
    [[nodiscard]] constexpr static u64 measure(const Array<T, N>& array) noexcept {
        u64 length = 2;
        for(u64 i = 0; i < N; i++) {
            length += Measure<T>::measure(array[i]);
            if(i + 1 < N) length += 2;
        }
        return length;
    }
};
template<Allocator O, Reflectable T, u64 N>
struct Write<O, Array<T, N>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Array<T, N>& array) noexcept {
        idx = output.write(idx, '[');
        for(u64 i = 0; i < N; i++) {
            idx = Write<O, T>::write(output, idx, array[i]);
            if(i + 1 < N) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, ']');
    }
};

} // namespace Format

template<typename T, u64 N>
struct Reflect::Refl<Array<T, N>> {
    using underlying = T;
    constexpr static Literal name = "Array";
    constexpr static Kind kind = Kind::array_;
    constexpr static u64 length = N;
};

} // namespace rpp