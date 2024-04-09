
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T, Allocator A = Mdefault>
struct Vec {

    Vec() noexcept = default;

    explicit Vec(u64 capacity) noexcept
        : data_(reinterpret_cast<T*>(A::alloc(capacity * sizeof(T)))), length_(0),
          capacity_(capacity) {
    }

    [[nodiscard]] static Vec make(u64 length) noexcept
        requires Default_Constructable<T>
    {
        Vec ret;
        ret.data_ = reinterpret_cast<T*>(A::alloc(length * sizeof(T)));
        new(ret.data_) T[length]{};
        ret.capacity_ = length;
        ret.length_ = length;
        return ret;
    }

    template<typename... Ss>
        requires All_Are<T, Ss...> && Move_Constructable<T>
    explicit Vec(Ss&&... init) noexcept {
        reserve(sizeof...(Ss));
        (push(rpp::move(init)), ...);
    }

    Vec(const Vec& src) noexcept = delete;
    Vec& operator=(const Vec& src) noexcept = delete;

    Vec(Vec&& src) noexcept {
        data_ = src.data_;
        length_ = src.length_;
        capacity_ = src.capacity_;
        src.data_ = null;
        src.length_ = 0;
        src.capacity_ = 0;
    }
    Vec& operator=(Vec&& src) noexcept {
        this->~Vec();
        data_ = src.data_;
        length_ = src.length_;
        capacity_ = src.capacity_;
        src.data_ = null;
        src.length_ = 0;
        src.capacity_ = 0;
        return *this;
    }

    ~Vec() noexcept {
        if constexpr(Must_Destruct<T>) {
            for(u64 i = 0; i < length_; i++) {
                data_[i].~T();
            }
        }
        A::free(data_);
        data_ = null;
        length_ = 0;
        capacity_ = 0;
    }

    template<Allocator B = A>
    [[nodiscard]] Vec<T, B> clone() const noexcept
        requires(Clone<T> || Copy_Constructable<T>)
    {
        Vec<T, B> ret(capacity_);
        ret.length_ = length_;
        if constexpr(Trivially_Copyable<T>) {
            Libc::memcpy(ret.data_, data_, length_ * sizeof(T));
        } else if constexpr(Clone<T>) {
            for(u64 i = 0; i < length_; i++) {
                new(&ret.data_[i]) T{data_[i].clone()};
            }
        } else {
            static_assert(Copy_Constructable<T>);
            for(u64 i = 0; i < length_; i++) {
                new(&ret.data_[i]) T{data_[i]};
            }
        }
        return ret;
    }

    void grow() noexcept {
        u64 new_capacity = capacity_ ? 2 * capacity_ : 8;
        reserve(new_capacity);
    }

    void clear() noexcept {
        if constexpr(Must_Destruct<T>) {
            for(u64 i = 0; i < length_; i++) {
                data_[i].~T();
            }
        }
        length_ = 0;
    }

    void reserve(u64 new_capacity) noexcept {
        if(new_capacity <= capacity_) return;

        T* new_data = reinterpret_cast<T*>(A::alloc(new_capacity * sizeof(T)));

        if(data_ && new_data) {
            if constexpr(Trivially_Movable<T>) {
                Libc::memcpy((void*)new_data, data_, sizeof(T) * length_);
            } else {
                static_assert(Move_Constructable<T>);
                for(u64 i = 0; i < length_; i++) {
                    new(&new_data[i]) T{rpp::move(data_[i])};
                }
            }
        }
        A::free(data_);

        capacity_ = new_capacity;
        data_ = new_data;
    }

    void extend(u64 additional_length) noexcept
        requires Default_Constructable<T>
    {
        resize(length_ + additional_length);
    }

    void resize(u64 new_length) noexcept
        requires Default_Constructable<T>
    {
        reserve(new_length);
        if(new_length > length_) {
            new(&data_[length_]) T[new_length - length_]{};
        } else if constexpr(Must_Destruct<T>) {
            for(u64 i = new_length; i < length_; i++) {
                data_[i].~T();
            }
        }
        length_ = new_length;
    }

    [[nodiscard]] bool empty() const noexcept {
        return length_ == 0;
    }
    [[nodiscard]] bool full() const noexcept {
        return length_ == capacity_;
    }
    void unsafe_fill() noexcept {
        length_ = capacity_;
    }

    T& push(const T& value) noexcept
        requires Copy_Constructable<T>
    {
        return push(T{value});
    }

    T& push(T&& value) noexcept
        requires Move_Constructable<T>
    {
        if(full()) grow();
        assert(length_ < capacity_);
        new(&data_[length_]) T{rpp::move(value)};
        return data_[length_++];
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    T& emplace(Args&&... args) noexcept {
        if(full()) grow();
        assert(length_ < capacity_);
        new(&data_[length_]) T{rpp::forward<Args>(args)...};
        return data_[length_++];
    }

    void pop() noexcept {
        assert(length_ > 0);
        length_--;
        if constexpr(Must_Destruct<T>) {
            data_[length_].~T();
        }
    }

    [[nodiscard]] T& front() noexcept {
        assert(length_ > 0);
        return data_[0];
    }
    [[nodiscard]] const T& front() const noexcept {
        assert(length_ > 0);
        return data_[0];
    }

    [[nodiscard]] T& back() noexcept {
        assert(length_ > 0);
        return data_[length_ - 1];
    }
    [[nodiscard]] const T& back() const noexcept {
        assert(length_ > 0);
        return data_[length_ - 1];
    }

    [[nodiscard]] T& operator[](u64 idx) noexcept {
        assert(idx < length_);
        return data_[idx];
    }
    [[nodiscard]] const T& operator[](u64 idx) const noexcept {
        assert(idx < length_);
        return data_[idx];
    }

    [[nodiscard]] const T* begin() const noexcept {
        return data_;
    }
    [[nodiscard]] const T* end() const noexcept {
        return data_ + length_;
    }
    [[nodiscard]] T* begin() noexcept {
        return data_;
    }
    [[nodiscard]] T* end() noexcept {
        return data_ + length_;
    }

    [[nodiscard]] u64 length() const noexcept {
        return length_;
    }
    [[nodiscard]] u64 capacity() const noexcept {
        return capacity_;
    }
    [[nodiscard]] u64 bytes() const noexcept {
        return length_ * sizeof(T);
    }

    [[nodiscard]] T* data() noexcept {
        return data_;
    }
    [[nodiscard]] const T* data() const noexcept {
        return data_;
    }

    [[nodiscard]] Slice<T> slice() const noexcept {
        return Slice<T>{data_, length_};
    }

private:
    T* data_ = null;
    u64 length_ = 0;
    u64 capacity_ = 0;

    friend struct Reflect::Refl<Vec>;
};

template<typename T, Allocator A>
RPP_TEMPLATE_RECORD(Vec, RPP_PACK(T, A), RPP_FIELD(data_), RPP_FIELD(length_),
                    RPP_FIELD(capacity_));

namespace Format {

template<Reflectable T, Allocator A>
struct Measure<Vec<T, A>> {
    [[nodiscard]] constexpr static u64 measure(const Vec<T, A>& vec) noexcept {
        u64 length = 5;
        for(u64 i = 0; i < vec.length(); i++) {
            length += Measure<T>::measure(vec[i]);
            if(i + 1 < vec.length()) length += 2;
        }
        return length;
    }
};

template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Vec<T, A>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Vec<T, A>& vec) noexcept {
        idx = output.write(idx, "Vec["_v);
        for(u64 i = 0; i < vec.length(); i++) {
            idx = Write<O, T>::write(output, idx, vec[i]);
            if(i + 1 < vec.length()) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, ']');
    }
};

} // namespace Format

} // namespace rpp
