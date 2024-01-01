
#pragma once

#include "base.h"

namespace rpp {

template<typename T, Scalar_Allocator P = Mdefault>
struct Box {
    using A = Pool_Adaptor<P>;

    Box() = default;

    explicit Box(T&& value)
        requires Move_Constructable<T>
    {
        data_ = A::template make<T>(move(value));
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    explicit Box(Args&&... args) {
        data_ = A::template make<T>(forward<Args>(args)...);
    }

    template<Base_Of<T> D>
    explicit Box(Box<D, P>&& derived) {
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
            A::template destroy<T>(data_);
            data_ = null;
        }
    }

    template<Scalar_Allocator R = P>
    Box<T, R> clone() const
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
        data_ = A::template make<T>(forward<Args>(args)...);
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

    template<typename T2, Scalar_Allocator P2>
    friend struct Box;
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
