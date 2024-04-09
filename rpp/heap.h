
#pragma once

#include "base.h"

namespace rpp {

template<typename T>
concept Heapable = Ordered<T> && Move_Constructable<T>;

template<Heapable T, Allocator A>
struct Heap;

template<Heapable T, Allocator A = Mdefault>
Heap(T...) -> Heap<T, A>;

template<Heapable T, Allocator A = Mdefault>
struct Heap {

    Heap() noexcept = default;

    explicit Heap(u64 capacity) noexcept {
        data_ = reinterpret_cast<T*>(A::alloc(capacity * sizeof(T)));
        length_ = 0;
        capacity_ = capacity;
    }

    template<typename... S>
        requires All_Are<T, S...> && Move_Constructable<T>
    explicit Heap(S&&... init) noexcept {
        reserve(sizeof...(S));
        (push(rpp::forward<S>(init)), ...);
    }

    explicit Heap(const Heap& src) noexcept = delete;
    Heap& operator=(const Heap& src) noexcept = delete;

    Heap(Heap&& src) noexcept {
        data_ = src.data_;
        length_ = src.length_;
        capacity_ = src.capacity_;
        src.data_ = null;
        src.length_ = 0;
        src.capacity_ = 0;
    }
    Heap& operator=(Heap&& src) noexcept {
        this->~Heap();
        data_ = src.data_;
        length_ = src.length_;
        capacity_ = src.capacity_;
        src.data_ = null;
        src.length_ = 0;
        src.capacity_ = 0;
        return *this;
    }

    ~Heap() noexcept {
        if constexpr(Must_Destruct<T>) {
            for(T& v : *this) {
                v.~T();
            }
        }
        A::free(data_);
        data_ = null;
        length_ = 0;
        capacity_ = 0;
    }

    template<Allocator B = A>
    [[nodiscard]] Heap<T, B> clone() const noexcept
        requires(Clone<T> || Copy_Constructable<T>)
    {
        Heap<T, B> ret;
        ret.data_ = reinterpret_cast<T*>(B::alloc(capacity_ * sizeof(T)));
        ret.length_ = length_;
        ret.capacity_ = capacity_;
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

    void reserve(u64 new_capacity) noexcept
        requires Trivially_Movable<T> || Move_Constructable<T>
    {
        if(new_capacity <= capacity_) return;

        T* new_data = reinterpret_cast<T*>(A::alloc(new_capacity * sizeof(T)));
        if constexpr(Trivially_Movable<T>) {
            Libc::memcpy(new_data, data_, length_ * sizeof(T));
        } else {
            static_assert(Move_Constructable<T>);
            for(u64 i = 0; i < length_; i++) {
                new(&new_data[i]) T{rpp::move(data_[i])};
            }
        }
        A::free(data_);

        capacity_ = new_capacity;
        data_ = new_data;
    }

    void grow() noexcept
        requires Trivially_Movable<T> || Move_Constructable<T>
    {
        u64 new_capacity = capacity_ ? 2 * capacity_ : 8;
        reserve(new_capacity);
    }

    void clear() noexcept {
        if constexpr(Must_Destruct<T>) {
            for(T& v : *this) {
                v.~T();
            }
        }
        length_ = 0;
    }

    [[nodiscard]] bool empty() const noexcept {
        return length_ == 0;
    }
    [[nodiscard]] bool full() const noexcept {
        return length_ == capacity_;
    }
    [[nodiscard]] u64 length() const noexcept {
        return length_;
    }

    void push(T&& value) noexcept
        requires Move_Constructable<T>
    {
        if(full()) grow();
        new(&data_[length_++]) T{rpp::move(value)};
        reheap_up(length_ - 1);
    }

    template<typename... Args>
    void emplace(Args&&... args) noexcept
        requires Constructable<T, Args...>
    {
        if(full()) grow();
        new(&data_[length_++]) T{rpp::forward<Args>(args)...};
        reheap_up(length_ - 1);
    }

    void pop() noexcept
        requires Trivially_Movable<T> || Move_Constructable<T>
    {
        assert(length_ > 0);

        if constexpr(Must_Destruct<T>) {
            data_[0].~T();
        }
        length_--;

        if(length_ > 0) {
            if constexpr(Trivially_Movable<T>) {
                Libc::memcpy(data_, data_ + length_, sizeof(T));
            } else {
                static_assert(Move_Constructable<T>);
                new(data_) T{rpp::move(data_[length_])};
            }
            reheap_down(0);
        }
    }

    [[nodiscard]] T& top() noexcept {
        return data_[0];
    }
    [[nodiscard]] const T& top() const noexcept {
        return data_[0];
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

private:
    void swap(u64 a, u64 b) noexcept
        requires Move_Constructable<T>
    {
        T temp{rpp::move(data_[a])};
        new(&data_[a]) T{rpp::move(data_[b])};
        new(&data_[b]) T{rpp::move(temp)};
    }

    void reheap_up(u64 idx) noexcept
        requires Move_Constructable<T>
    {
        while(idx) {
            u64 parent_idx = (idx - 1) / 2;
            T& elem = data_[idx];
            T& parent = data_[parent_idx];
            if(elem < parent) {
                swap(idx, parent_idx);
                idx = parent_idx;
            } else {
                return;
            }
        }
    }

    void reheap_down(u64 idx) noexcept
        requires Move_Constructable<T>
    {
        while(true) {
            T& parent = data_[idx];

            u64 left = idx * 2 + 1;
            u64 right = left + 1;

            if(right < length_) {
                T& lchild = data_[left];
                T& rchild = data_[right];
                if(lchild < parent && !(rchild < lchild)) {
                    swap(idx, left);
                    idx = left;
                } else if(rchild < parent && !(lchild < rchild)) {
                    swap(idx, right);
                    idx = right;
                } else {
                    return;
                }
            } else if(left < length_) {
                T& lchild = data_[left];
                if(lchild < parent) {
                    swap(idx, left);
                }
                return;
            } else {
                return;
            }
        }
    }

    T* data_ = null;
    u64 length_ = 0;
    u64 capacity_ = 0;

    friend struct Reflect::Refl<Heap>;
};

template<typename T, Allocator A>
RPP_TEMPLATE_RECORD(Heap, RPP_PACK(T, A), RPP_FIELD(data_), RPP_FIELD(length_),
                    RPP_FIELD(capacity_));

namespace Format {

template<Reflectable T, Allocator A>
struct Measure<Heap<T, A>> {
    [[nodiscard]] static u64 measure(const Heap<T, A>& heap) noexcept {
        u64 n = 0;
        u64 length = 6;
        for(const T& item : heap) {
            length += Measure<T>::measure(item);
            if(n + 1 < heap.length()) length += 2;
            n++;
        }
        return length;
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Heap<T, A>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Heap<T, A>& heap) noexcept {
        idx = output.write(idx, "Heap["_v);
        u64 n = 0;
        for(const T& item : heap) {
            idx = Write<O, T>::write(output, idx, item);
            if(n + 1 < heap.length()) idx = output.write(idx, ", "_v);
            n++;
        }
        return output.write(idx, ']');
    }
};

} // namespace Format

} // namespace rpp
