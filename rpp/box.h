
#pragma once

#include "base.h"

namespace rpp {

template<typename T, Scalar_Allocator P = Mdefault>
struct Box {
    using A = Pool_Adaptor<P>;

    Box() = default;

    explicit Box(T&& value) noexcept
        requires Move_Constructable<T>
    {
        data_ = A::template make<T>(move(value));
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    explicit Box(Args&&... args) noexcept {
        data_ = A::template make<T>(forward<Args>(args)...);
    }

    template<Base_Of<T> D>
    explicit Box(Box<D, P>&& derived) noexcept {
        data_ = derived.data_;
        derived.data_ = null;
    }

    Box(const Box& src) noexcept = delete;
    Box& operator=(const Box& src) noexcept = delete;

    Box(Box&& src) noexcept {
        data_ = src.data_;
        src.data_ = null;
    }
    Box& operator=(Box&& src) noexcept {
        this->~Box();
        data_ = src.data_;
        src.data_ = null;
        return *this;
    }

    ~Box() noexcept {
        if(data_) {
            A::template destroy<T>(data_);
            data_ = null;
        }
    }

    template<Scalar_Allocator R = P>
    [[nodiscard]] Box<T, R> clone() const noexcept
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
    void emplace(Args&&... args) noexcept {
        this->~Box();
        data_ = A::template make<T>(forward<Args>(args)...);
    }

    [[nodiscard]] operator bool() const noexcept {
        return data_ != null;
    }
    [[nodiscard]] T* operator->() noexcept {
        assert(data_);
        return data_;
    }
    [[nodiscard]] const T* operator->() const noexcept {
        assert(data_);
        return data_;
    }

    [[nodiscard]] T& operator*() noexcept {
        assert(data_);
        return *data_;
    }
    [[nodiscard]] const T& operator*() const noexcept {
        assert(data_);
        return *data_;
    }

private:
    T* data_ = null;

    template<typename T2, Scalar_Allocator P2>
    friend struct Box;
    friend struct Reflect::Refl<Box>;
};

template<typename T, Scalar_Allocator P>
RPP_TEMPLATE_RECORD(Box, RPP_PACK(T, P), RPP_FIELD(data_));

namespace Format {

template<Reflectable T, typename A>
struct Measure<Box<T, A>> {
    [[nodiscard]] static u64 measure(const Box<T, A>& box) noexcept {
        if(box) return 5 + Measure<T>::measure(*box);
        return 9;
    }
};
template<Allocator O, Reflectable T, typename A>
struct Write<O, Box<T, A>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Box<T, A>& box) noexcept {
        if(!box) return output.write(idx, "Box{null}"_v);
        idx = output.write(idx, "Box{"_v);
        idx = Write<O, T>::write(output, idx, *box);
        return output.write(idx, '}');
    }
};

} // namespace Format

} // namespace rpp
