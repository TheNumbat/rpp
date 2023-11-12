
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T, u64 N>
    requires(N > 0)
struct Array {
    static constexpr u64 capacity = N;

    Array()
        requires Default_Constructable<T>
    = default;

    template<typename... Ss>
        requires(sizeof...(Ss) == N) && All<T, Ss...>
    explicit Array(Ss&&... init) : data_{std::forward<Ss>(init)...} {
    }

    template<typename... Ss>
        requires(sizeof...(Ss) == N) && All<T, Ss...> && Copy_Constructable<T>
    explicit Array(const Ss&... init) : data_{T{init}...} {
    }

    ~Array() = default;

    Array(const Array& src)
        requires Copy_Constructable<T>
    = default;
    Array& operator=(const Array& src)
        requires Copy_Constructable<T>
    = default;

    Array(Array&& src) = default;
    Array& operator=(Array&& src) = default;

    Array clone() const
        requires(Clone<T> || Copy_Constructable<T>) && Default_Constructable<T>
    {
        Array result;
        if constexpr(Clone<T>) {
            for(u64 i = 0; i < N; i++) {
                result[i] = data_[i].clone();
            }
        } else {
            Std::memcpy(result.data_, data_, sizeof(T) * N);
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
    friend struct rpp::detail::Reflect<Array>;
};

template<typename T, u64 N>
struct rpp::detail::Reflect<Array<T, N>> {
    using underlying = T;
    static constexpr Literal name = "Array";
    static constexpr Kind kind = Kind::array_;
    static constexpr u64 length = N;
};

namespace Format {

template<Reflectable T, u64 N>
struct Measure<Array<T, N>> {
    static u64 measure(const Array<T, N>& array) {
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
    static u64 write(String<O>& output, u64 idx, const Array<T, N>& array) {
        idx = output.write(idx, '[');
        for(u64 i = 0; i < N; i++) {
            idx = Write<O, T>::write(output, idx, array[i]);
            if(i + 1 < N) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, ']');
    }
};

} // namespace Format

} // namespace rpp