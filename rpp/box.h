
#pragma once

namespace rpp {

template<typename T, Allocator A = Mdefault>
struct Box {

    Box() = default;

    explicit Box(T&& value)
        requires Move_Constructable<T>
    {
        data_ = reinterpret_cast<T*>(A::alloc(sizeof(T)));
        new(data_) T{std::move(value)};
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    explicit Box(Args&&... args) {
        data_ = reinterpret_cast<T*>(A::alloc(sizeof(T)));
        new(data_) T{std::forward<Args>(args)...};
    }

    template<Derived_From<T> D>
    explicit Box(Box<D, A>&& derived) {
        data_ = derived.data_;
        derived.data_ = null;
    }

    Box(const Box& src) = delete;
    Box& operator=(const Box& src) = delete;

    Box(Box&& src) {
        data_ = src.data_;
        src.data_ = null;
    }
    Box& operator=(Box&& src) {
        this->~Box();
        data_ = src.data_;
        src.data_ = null;
        return *this;
    }

    ~Box() {
        if(data_) {
            if constexpr(Must_Destruct<T>) {
                data_->~T();
            }
            A::free(data_);
        }
    }

    template<Allocator B = A>
    Box<T, B> clone() const
        requires Clone<T>
    {
        if(!data_) return Box{};
        return Box{data_->clone()};
    }

    template<Allocator B = A>
    Box<T, B> clone() const
        requires Trivial<T>
    {
        if(!data_) return Box{};
        return Box{*data_};
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        if(data_) {
            if constexpr(Must_Destruct<T>) {
                data_->~T();
            }
        } else {
            data_ = reinterpret_cast<T*>(A::alloc(sizeof(T)));
        }
        new(data_) T{std::forward<Args>(args)...};
    }

    operator bool() const {
        return data_ != null;
    }
    T* operator->() {
        assert(data_);
        return data_;
    }
    const T* operator->() const {
        assert(data_);
        return data_;
    }

    T& operator*() {
        assert(data_);
        return *data_;
    }
    const T& operator*() const {
        assert(data_);
        return *data_;
    }

private:
    T* data_ = null;

    friend struct Reflect<Box>;
};

template<typename B, Allocator A>
struct Reflect<Box<B, A>> {
    using T = Box<B, A>;
    static constexpr Literal name = "Box";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_)>;
    static_assert(Record<T>);
};

} // namespace rpp
