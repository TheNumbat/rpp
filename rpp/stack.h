
#pragma once

#include "base.h"

namespace rpp {

template<Movable T, Allocator A = Mdefault>
struct Stack {

    Stack() = default;
    explicit Stack(u64 capacity) : data_(capacity) {
    }

    template<typename... S>
        requires All_Are<T, S...> && Move_Constructable<T>
    explicit Stack(S&&... init) : data_(forward<S>(init)...) {
    }

    Stack(const Stack& src) = delete;
    Stack& operator=(const Stack& src) = delete;

    Stack(Stack&& src) = default;
    Stack& operator=(Stack&& src) = default;

    ~Stack() = default;

    template<Allocator B = A>
    Stack<T, B> clone() const
        requires Clone<T> || Copy_Constructable<T>
    {
        Stack<T, B> ret;
        ret.data_ = data_.clone();
        return ret;
    }

    u64 length() const {
        return data_.length();
    }
    bool empty() const {
        return data_.empty();
    }
    bool full() const {
        return data_.full();
    }

    T& push(const T& value)
        requires Copy_Constructable<T>
    {
        return push(T{value});
    }

    T& push(T&& value)
        requires Move_Constructable<T>
    {
        return data_.push(move(value));
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    T& emplace(Args&&... args) {
        return data_.emplace(forward<Args>(args)...);
    }

    void pop() {
        return data_.pop();
    }

    T& top() {
        return data_.back();
    }
    const T& top() const {
        return data_.back();
    }

    void clear() {
        data_.clear();
    }

    const T* begin() const {
        return data_.begin();
    }
    const T* end() const {
        return data_.end();
    }
    T* begin() {
        return data_.begin();
    }
    T* end() {
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
    static u64 measure(const Stack<T, A>& stack) {
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
    static u64 write(String<O>& output, u64 idx, const Stack<T, A>& stack) {
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
