
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T>
struct Slice;

template<typename T, Allocator A>
struct Vec;

template<typename T, u64 N>
    requires(N > 0)
struct Array;

template<typename T, Allocator A>
Slice(Vec<T, A>) -> Slice<T>;

template<typename T, u64 N>
Slice(Array<T, N>) -> Slice<T>;

template<typename T>
Slice(std::initializer_list<T>) -> Slice<T>;

template<typename T>
Slice(const T*, u64) -> Slice<T>;

template<typename T>
struct Slice {

    constexpr Slice() noexcept = default;

    template<Allocator A>
    explicit Slice(const Vec<T, A>& v) noexcept {
        data_ = v.data();
        length_ = v.length();
    }

    template<u64 N>
    constexpr explicit Slice(const Array<T, N>& a) noexcept {
        data_ = a.data();
        length_ = a.length();
    }

    constexpr explicit Slice(const T* data, u64 length) noexcept {
        data_ = data;
        length_ = length;
    }

    constexpr explicit Slice(std::initializer_list<T> init) noexcept {
        data_ = init.begin();
        length_ = init.size();
    }

    constexpr Slice(const Slice& src) noexcept = default;
    constexpr Slice& operator=(const Slice& src) noexcept = default;

    constexpr Slice(Slice&& src) noexcept = default;
    constexpr Slice& operator=(Slice&& src) noexcept = default;

    constexpr ~Slice() noexcept = default;

    [[nodiscard]] constexpr Slice clone() const noexcept {
        return Slice{data_, length_};
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return length_ == 0;
    }

    [[nodiscard]] constexpr const T& front() const noexcept {
        assert(length_ > 0);
        return data_[0];
    }
    [[nodiscard]] constexpr const T& back() const noexcept {
        assert(length_ > 0);
        return data_[length_ - 1];
    }

    [[nodiscard]] constexpr const T& operator[](u64 idx) const noexcept {
        assert(idx < length_);
        return data_[idx];
    }

    [[nodiscard]] constexpr const T* data() const noexcept {
        return data_;
    }

    [[nodiscard]] constexpr const T* begin() const noexcept {
        return data_;
    }
    [[nodiscard]] constexpr const T* end() const noexcept {
        return data_ + length_;
    }

    [[nodiscard]] constexpr u64 length() const noexcept {
        return length_;
    }
    [[nodiscard]] constexpr u64 bytes() const noexcept {
        return length_ * sizeof(T);
    }

    [[nodiscard]] constexpr Slice sub(u64 start, u64 length) const noexcept {
        assert(start + length <= length_);
        return Slice{data_ + start, length};
    }

    [[nodiscard]] Slice<u8> to_bytes() noexcept {
        Slice<u8> ret;
        ret.data_ = reinterpret_cast<const u8*>(data_);
        ret.length_ = length_ * sizeof(T);
        return ret;
    }

private:
    const T* data_ = null;
    u64 length_ = 0;

    template<typename>
    friend struct Slice;
    friend struct Reflect::Refl<Slice<T>>;
};

template<typename T>
RPP_TEMPLATE_RECORD(Slice, T, RPP_FIELD(data_), RPP_FIELD(length_));

namespace Format {

template<Reflectable T>
struct Measure<Slice<T>> {
    [[nodiscard]] static u64 measure(const Slice<T>& slice) noexcept {
        u64 length = 7;
        for(u64 i = 0; i < slice.length(); i++) {
            length += Measure<T>::measure(slice[i]);
            if(i + 1 < slice.length()) length += 2;
        }
        return length;
    }
};

template<Allocator O, Reflectable T>
struct Write<O, Slice<T>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Slice<T>& slice) noexcept {
        idx = output.write(idx, "Slice["_v);
        for(u64 i = 0; i < slice.length(); i++) {
            idx = Write<O, T>::write(output, idx, slice[i]);
            if(i + 1 < slice.length()) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, ']');
    }
};

} // namespace Format

} // namespace rpp