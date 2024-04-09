
#pragma once

#include "base.h"

namespace rpp {

template<Movable T, Allocator A>
struct Stack;

template<Movable T, Allocator A = Mdefault>
Stack(T...) -> Stack<T, A>;

template<Movable T, Allocator A = Mdefault>
struct Stack {

    Stack() noexcept = default;
    explicit Stack(u64 capacity) noexcept : data_(capacity) {
    }

    template<typename... S>
        requires All_Are<T, S...> && Move_Constructable<T>
    explicit Stack(S&&... init) noexcept : data_(rpp::forward<S>(init)...) {
    }

    Stack(const Stack& src) noexcept = delete;
    Stack& operator=(const Stack& src) noexcept = delete;

    Stack(Stack&& src) noexcept = default;
    Stack& operator=(Stack&& src) noexcept = default;

    ~Stack() noexcept = default;

    template<Allocator B = A>
    [[nodiscard]] Stack<T, B> clone() const noexcept
        requires Clone<T> || Copy_Constructable<T>
    {
        Stack<T, B> ret;
        ret.data_ = data_.clone();
        return ret;
    }

    [[nodiscard]] u64 length() const noexcept {
        return data_.length();
    }
    [[nodiscard]] bool empty() const noexcept {
        return data_.empty();
    }
    [[nodiscard]] bool full() const noexcept {
        return data_.full();
    }

    T& push(const T& value) noexcept
        requires Copy_Constructable<T>
    {
        return push(T{value});
    }

    T& push(T&& value) noexcept
        requires Move_Constructable<T>
    {
        return data_.push(rpp::move(value));
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    T& emplace(Args&&... args) noexcept {
        return data_.emplace(rpp::forward<Args>(args)...);
    }

    void pop() noexcept {
        return data_.pop();
    }

    [[nodiscard]] T& top() noexcept {
        return data_.back();
    }
    [[nodiscard]] const T& top() const noexcept {
        return data_.back();
    }

    void clear() noexcept {
        data_.clear();
    }

    [[nodiscard]] const T* begin() const noexcept {
        return data_.begin();
    }
    [[nodiscard]] const T* end() const noexcept {
        return data_.end();
    }
    [[nodiscard]] T* begin() noexcept {
        return data_.begin();
    }
    [[nodiscard]] T* end() noexcept {
        return data_.end();
    }

private:
    Vec<T, A> data_;

    friend struct Reflect::Refl<Stack>;
};

template<typename T, Allocator A>
RPP_TEMPLATE_RECORD(Stack, RPP_PACK(T, A), RPP_FIELD(data_));

namespace Format {

template<Reflectable T, Allocator A>
struct Measure<Stack<T, A>> {
    [[nodiscard]] static u64 measure(const Stack<T, A>& stack) noexcept {
        u64 n = 0;
        u64 length = 7;
        for(const T& item : stack) {
            length += Measure<T>::measure(item);
            if(n + 1 < stack.length()) length += 2;
            n++;
        }
        return length;
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Stack<T, A>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Stack<T, A>& stack) noexcept {
        idx = output.write(idx, "Stack["_v);
        u64 n = 0;
        for(const T& item : stack) {
            idx = Write<O, T>::write(output, idx, item);
            if(n + 1 < stack.length()) idx = output.write(idx, ", "_v);
            n++;
        }
        return output.write(idx, ']');
    }
};

} // namespace Format

} // namespace rpp
