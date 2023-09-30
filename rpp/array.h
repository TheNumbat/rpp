
#pragma once

namespace rpp {

template<typename T, u64 N>
    requires(N > 0)
struct Array {
    static constexpr u64 capacity = N;

    Array()
        requires Default_Constructable<T>
    = default;

    template<typename... Ss>
        requires Length<N, Ss...> && All<T, Ss...> && Move_Constructable<T>
    explicit Array(Ss&&... init) : data_{std::move(init)...} {
    }

    ~Array() = default;

    Array(const Array& src)
        requires Trivial<T>
    = default;
    Array& operator=(const Array& src)
        requires Trivial<T>
    = default;

    Array(Array&& src) = default;
    Array& operator=(Array&& src) = default;

    Array clone() const
        requires(Clone<T> || Trivial<T>) && Default_Constructable<T>
    {
        Array result;
        if constexpr(Clone<T>) {
            for(u64 i = 0; i < N; i++) {
                result[i] = data_[i].clone();
            }
        } else {
            std::memcpy(result.data_, data_, sizeof(T) * N);
        }
        return result;
    }

    T& operator[](u64 idx) {
        assert(idx < N);
        return data_[idx];
    }
    const T& operator[](u64 idx) const {
        assert(idx < N);
        return data_[idx];
    }

    T* data() {
        return data_;
    }
    const T* data() const {
        return data_;
    }

    constexpr u64 length() const {
        return capacity;
    }

    const T* begin() const {
        return data_;
    }
    const T* end() const {
        return data_ + N;
    }
    T* begin() {
        return data_;
    }
    T* end() {
        return data_ + N;
    }

private:
    T data_[N] = {};
    friend struct Reflect<Array>;
};

template<typename T, u64 N>
struct Reflect<Array<T, N>> {
    using underlying = T;
    static constexpr Literal name = "Array";
    static constexpr Kind kind = Kind::array_;
    static constexpr u64 length = N;
};

} // namespace rpp