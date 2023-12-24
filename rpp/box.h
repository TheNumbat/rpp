
#pragma once

#include "base.h"

namespace rpp {

template<typename T, typename A = Mdefault>
    requires Pool_Allocator<A, T>
struct Box {

    Box() = default;

    explicit Box(T&& value)
        requires Move_Constructable<T>
    {
        data_ = make(move(value));
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    explicit Box(Args&&... args) {
        data_ = make(forward<Args>(args)...);
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
            destroy(data_);
            data_ = null;
        }
    }

    template<Allocator B = A>
    Box<T, B> clone() const
        requires(Clone<T> || Copy_Constructable<T>)
    {
        if(!data_) return Box{};
        if constexpr(Clone<T>) {
            return Box{data_->clone()};
        } else {
            static_assert(Copy_Constructable<T>);
            return Box{*data_};
        }
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        this->~Box();
        data_ = make(forward<Args>(args)...);
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
    template<typename... Args>
        requires Constructable<T, Args...>
    static T* make(Args&&... args) {
        if constexpr(Allocator<A>) {
            T* data_ = reinterpret_cast<T*>(A::alloc(sizeof(T)));
            new(data_) T{forward<Args>(args)...};
            return data_;
        } else {
            static_assert(Pool<A, T>);
            return A::template make<T>(forward<Args>(args)...);
        }
    }

    static void destroy(T* value) {
        if constexpr(Allocator<A>) {
            if constexpr(Must_Destruct<T>) {
                value->~T();
            }
            A::free(value);
        } else {
            static_assert(Pool<A, T>);
            A::template destroy<T>(value);
        }
    }

    T* data_ = null;

    friend struct Reflect::Refl<Box>;
};

namespace Reflect {

template<typename B, typename A>
struct Refl<Box<B, A>> {
    using T = Box<B, A>;
    static constexpr Literal name = "Box";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_)>;
};

} // namespace Reflect

namespace Format {

template<Reflectable T, typename A>
struct Measure<Box<T, A>> {
    static u64 measure(const Box<T, A>& box) {
        if(box) return 5 + Measure<T>::measure(*box);
        return 9;
    }
};
template<Allocator O, Reflectable T, typename A>
struct Write<O, Box<T, A>> {
    static u64 write(String<O>& output, u64 idx, const Box<T, A>& box) {
        if(!box) return output.write(idx, "Box{null}"_v);
        idx = output.write(idx, "Box{"_v);
        idx = Write<O, T>::write(output, idx, *box);
        return output.write(idx, '}');
    }
};

} // namespace Format

} // namespace rpp
