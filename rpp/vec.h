
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T>
struct Slice;

template<typename T, Allocator A = Mdefault>
struct Vec {

    Vec() = default;
    explicit Vec(u64 capacity)
        : data_(reinterpret_cast<T*>(A::alloc(capacity * sizeof(T)))), length_(0),
          capacity_(capacity) {
    }

    static Vec make(u64 length)
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
    explicit Vec(Ss&&... init) {
        reserve(sizeof...(Ss));
        (push(move(init)), ...);
    }

    Vec(const Vec& src) = delete;
    Vec& operator=(const Vec& src) = delete;

    Vec(Vec&& src) {
        data_ = src.data_;
        length_ = src.length_;
        capacity_ = src.capacity_;
        src.data_ = null;
        src.length_ = 0;
        src.capacity_ = 0;
    }
    Vec& operator=(Vec&& src) {
        this->~Vec();
        data_ = src.data_;
        length_ = src.length_;
        capacity_ = src.capacity_;
        src.data_ = null;
        src.length_ = 0;
        src.capacity_ = 0;
        return *this;
    }

    ~Vec() {
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
    Vec<T, B> clone() const
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

    void grow() {
        u64 new_capacity = capacity_ ? 2 * capacity_ : 8;
        reserve(new_capacity);
    }

    void clear() {
        if constexpr(Must_Destruct<T>) {
            for(u64 i = 0; i < length_; i++) {
                data_[i].~T();
            }
        }
        length_ = 0;
    }

    void reserve(u64 new_capacity) {
        if(new_capacity <= capacity_) return;

        T* new_data = reinterpret_cast<T*>(A::alloc(new_capacity * sizeof(T)));

        if(data_ && new_data) {
            if constexpr(Trivially_Movable<T>) {
                Libc::memcpy((void*)new_data, data_, sizeof(T) * length_);
            } else {
                static_assert(Move_Constructable<T>);
                for(u64 i = 0; i < length_; i++) {
                    new(&new_data[i]) T{move(data_[i])};
                }
            }
        }
        A::free(data_);

        capacity_ = new_capacity;
        data_ = new_data;
    }

    void extend(u64 additional_length)
        requires Default_Constructable<T>
    {
        resize(length_ + additional_length);
    }

    void resize(u64 new_length)
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

    bool empty() const {
        return length_ == 0;
    }
    bool full() const {
        return length_ == capacity_;
    }
    void unsafe_fill() {
        length_ = capacity_;
    }

    T& push(const T& value)
        requires Copy_Constructable<T>
    {
        return push(T{value});
    }

    T& push(T&& value)
        requires Move_Constructable<T>
    {
        if(full()) grow();
        assert(length_ < capacity_);
        new(&data_[length_]) T{move(value)};
        return data_[length_++];
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    T& emplace(Args&&... args) {
        if(full()) grow();
        assert(length_ < capacity_);
        new(&data_[length_]) T{rpp::forward<Args>(args)...};
        return data_[length_++];
    }

    void pop() {
        assert(length_ > 0);
        length_--;
        if constexpr(Must_Destruct<T>) {
            data_[length_].~T();
        }
    }

    T& front() {
        assert(length_ > 0);
        return data_[0];
    }
    const T& front() const {
        assert(length_ > 0);
        return data_[0];
    }

    T& back() {
        assert(length_ > 0);
        return data_[length_ - 1];
    }
    const T& back() const {
        assert(length_ > 0);
        return data_[length_ - 1];
    }

    T& operator[](u64 idx) {
        assert(idx < length_);
        return data_[idx];
    }
    const T& operator[](u64 idx) const {
        assert(idx < length_);
        return data_[idx];
    }

    const T* begin() const {
        return data_;
    }
    const T* end() const {
        return data_ + length_;
    }
    T* begin() {
        return data_;
    }
    T* end() {
        return data_ + length_;
    }

    u64 length() const {
        return length_;
    }
    u64 capacity() const {
        return capacity_;
    }
    u64 bytes() const {
        return length_ * sizeof(T);
    }

    T* data() {
        return data_;
    }
    const T* data() const {
        return data_;
    }

    Slice<T> slice() const {
        return Slice<T>{data_, length_};
    }

private:
    T* data_ = null;
    u64 length_ = 0;
    u64 capacity_ = 0;

    friend struct Reflect::Refl<Vec>;
};

template<typename T>
struct Slice {

    Slice() = default;

    template<Allocator A>
    explicit Slice(const Vec<T, A>& v) {
        data_ = v.data();
        length_ = v.length();
    }
    template<u64 N>
    explicit Slice(const Array<T, N>& a) {
        data_ = a.data();
        length_ = a.length();
    }
    explicit Slice(const T* data, u64 length) {
        data_ = data;
        length_ = length;
    }

    explicit Slice(std::initializer_list<T> init) {
        data_ = init.begin();
        length_ = init.size();
    }

    Slice(const Slice& src) = default;
    Slice& operator=(const Slice& src) = default;

    Slice(Slice&& src) = default;
    Slice& operator=(Slice&& src) = default;

    ~Slice() = default;

    Slice clone() const {
        return Slice{data_, length_};
    }

    bool empty() const {
        return length_ == 0;
    }

    const T& front() const {
        assert(length_ > 0);
        return data_[0];
    }
    const T& back() const {
        assert(length_ > 0);
        return data_[length_ - 1];
    }

    const T& operator[](u64 idx) const {
        assert(idx < length_);
        return data_[idx];
    }

    const T* data() const {
        return data_;
    }

    const T* begin() const {
        return data_;
    }
    const T* end() const {
        return data_ + length_;
    }

    u64 length() const {
        return length_;
    }
    u64 bytes() const {
        return length_ * sizeof(T);
    }

    Slice sub(u64 start, u64 length) const {
        assert(start + length <= length_);
        return Slice{data_ + start, length};
    }

    Slice<u8> to_bytes() {
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

namespace Reflect {

template<typename V, Allocator A>
struct Refl<Vec<V, A>> {
    using T = Vec<V, A>;
    static constexpr Literal name = "Vec";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_), FIELD(length_), FIELD(capacity_)>;
};

template<typename V>
struct Refl<Slice<V>> {
    using T = Slice<V>;
    static constexpr Literal name = "Slice";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_), FIELD(length_)>;
};

} // namespace Reflect

namespace Format {

template<Reflectable T, Allocator A>
struct Measure<Vec<T, A>> {
    static u64 measure(const Vec<T, A>& vec) {
        u64 length = 5;
        for(u64 i = 0; i < vec.length(); i++) {
            length += Measure<T>::measure(vec[i]);
            if(i + 1 < vec.length()) length += 2;
        }
        return length;
    }
};
template<Reflectable T>
struct Measure<Slice<T>> {
    static u64 measure(const Slice<T>& slice) {
        u64 length = 7;
        for(u64 i = 0; i < slice.length(); i++) {
            length += Measure<T>::measure(slice[i]);
            if(i + 1 < slice.length()) length += 2;
        }
        return length;
    }
};

template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Vec<T, A>> {
    static u64 write(String<O>& output, u64 idx, const Vec<T, A>& vec) {
        idx = output.write(idx, "Vec["_v);
        for(u64 i = 0; i < vec.length(); i++) {
            idx = Write<O, T>::write(output, idx, vec[i]);
            if(i + 1 < vec.length()) idx = output.write(idx, ", "_v);
        }
        return output.write(idx, ']');
    }
};
template<Allocator O, Reflectable T>
struct Write<O, Slice<T>> {
    static u64 write(String<O>& output, u64 idx, const Slice<T>& slice) {
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