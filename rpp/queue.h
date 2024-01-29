
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<Movable T, Allocator A = Mdefault>
struct Queue {

    Queue() noexcept = default;
    explicit Queue(u64 capacity) noexcept {
        data_ = reinterpret_cast<T*>(A::alloc(capacity * sizeof(T)));
        length_ = 0;
        last_ = 0;
        capacity_ = capacity;
    }

    template<typename... Ss>
        requires All_Are<T, Ss...> && Move_Constructable<T>
    explicit Queue(Ss&&... init) noexcept {
        reserve(sizeof...(Ss));
        (push(rpp::move(init)), ...);
    }

    Queue(const Queue& src) noexcept = delete;
    Queue& operator=(const Queue& src) noexcept = delete;

    Queue(Queue&& src) noexcept {
        data_ = src.data_;
        length_ = src.length_;
        last_ = src.last_;
        capacity_ = src.capacity_;
        src.data_ = null;
        src.length_ = 0;
        src.last_ = 0;
        src.capacity_ = 0;
    }
    Queue& operator=(Queue&& src) noexcept {
        this->~Queue();
        data_ = src.data_;
        length_ = src.length_;
        last_ = src.last_;
        capacity_ = src.capacity_;
        src.data_ = null;
        src.length_ = 0;
        src.last_ = 0;
        src.capacity_ = 0;
        return *this;
    }

    ~Queue() noexcept {
        clear();
        A::free(data_);
        data_ = null;
        capacity_ = 0;
    }

    template<Allocator B = A>
    [[nodiscard]] Queue<T, B> clone() const noexcept
        requires Clone<T> || Copy_Constructable<T>
    {
        Queue<T, B> ret;
        ret.data_ = reinterpret_cast<T*>(B::alloc(capacity_ * sizeof(T)));
        ret.length_ = length_;
        ret.last_ = last_;
        ret.capacity_ = capacity_;

        if constexpr(Trivially_Copyable<T>) {
            Libc::memcpy(ret.data_, data_, capacity_ * sizeof(T));
        } else if constexpr(Clone<T>) {
            u64 start = start_idx();
            u64 end = end_idx();
            for(u64 i = start; i != end; i = i == capacity_ - 1 ? 0 : i + 1) {
                new(&ret.data_[i]) T{data_[i].clone()};
            }
        } else {
            static_assert(Copy_Constructable<T>);
            u64 start = start_idx();
            u64 end = end_idx();
            for(u64 i = start; i != end; i = i == capacity_ - 1 ? 0 : i + 1) {
                new(&ret.data_[i]) T{data_[i]};
            }
        }
        return ret;
    }

    void grow() noexcept {
        u64 new_capacity = capacity_ ? 2 * capacity_ : 8;
        reserve(new_capacity);
    }

    void reserve(u64 new_capacity) noexcept {
        if(new_capacity <= capacity_) return;

        T* new_data = reinterpret_cast<T*>(A::alloc(new_capacity * sizeof(T)));
        T* start = data_ + last_ - length_;

        if constexpr(Trivially_Movable<T>) {
            if(length_ <= last_) {
                Libc::memcpy(new_data, start, sizeof(T) * length_);
            } else {
                u64 first = length_ - last_;
                Libc::memcpy(new_data, start + capacity_, sizeof(T) * first);
                Libc::memcpy(new_data + first, data_, sizeof(T) * last_);
            }
        } else {
            static_assert(Move_Constructable<T>);
            if(length_ <= last_) {
                for(u64 i = 0; i < length_; i++) {
                    new(&new_data[i]) T{rpp::move(start[i])};
                }
            } else {
                u64 first = length_ - last_;
                for(u64 i = 0; i < first; i++) {
                    new(&new_data[i]) T{rpp::move(start[i + capacity_])};
                }
                for(u64 i = 0; i < last_; i++) {
                    new(&new_data[first + i]) T{rpp::move(data_[i])};
                }
            }
        }

        A::free(data_);
        last_ = length_;
        capacity_ = new_capacity;
        data_ = new_data;
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

        new(&data_[last_]) T{rpp::move(value)};
        T& ret = data_[last_];

        length_++;
        last_ = last_ == capacity_ - 1 ? 0 : last_ + 1;
        return ret;
    }

    template<typename... Args>
    T& emplace(Args&&... args) noexcept
        requires Constructable<T, Args...>
    {
        if(full()) grow();

        new(&data_[last_]) T{rpp::forward<Args>(args)...};
        T& ret = data_[last_];

        length_++;
        last_ = last_ == capacity_ - 1 ? 0 : last_ + 1;
        return ret;
    }

    void pop() noexcept {
        assert(length_ > 0);
        u64 idx = length_ <= last_ ? last_ - length_ : last_ - length_ + capacity_;
        length_--;
        data_[idx].~T();
    }

    void clear() noexcept {
        if constexpr(Must_Destruct<T>) {
            for(T& value : *this) {
                value.~T();
            }
        }
        length_ = 0;
        last_ = 0;
    }

    [[nodiscard]] T& front() noexcept {
        assert(!empty());
        return data_[start_idx()];
    }
    [[nodiscard]] const T& front() const noexcept {
        assert(!empty());
        return data_[start_idx()];
    }

    [[nodiscard]] T& back() noexcept {
        assert(!empty());
        return data_[end_idx()];
    }
    [[nodiscard]] const T& back() const noexcept {
        assert(!empty());
        return data_[end_idx()];
    }

    [[nodiscard]] T& penultimate() noexcept {
        assert(length_ > 1);
        u64 idx = last_ == 0 ? capacity_ - 2 : last_ == 1 ? capacity_ - 1 : last_ - 2;
        return data_[idx];
    }
    [[nodiscard]] const T& penultimate() const noexcept {
        assert(length_ > 1);
        u64 idx = last_ == 0 ? capacity_ - 2 : last_ == 1 ? capacity_ - 1 : last_ - 2;
        return data_[idx];
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

    [[nodiscard]] T& operator[](u64 idx) noexcept {
        assert(idx < length_);
        u64 i = idx + start_idx();
        i = i >= capacity_ ? i - capacity_ : i;
        return data_[i];
    }
    [[nodiscard]] const T& operator[](u64 idx) const noexcept {
        assert(idx < length_);
        u64 i = idx + start_idx();
        i = i >= capacity_ ? i - capacity_ : i;
        return data_[i];
    }

    [[nodiscard]] bool erase_first(const T& value) noexcept
        requires Equality<T> && (Trivially_Movable<T> || Move_Constructable<T>)
    {
        bool found = false;
        u64 idx = start_idx();
        u64 consumed = 0;
        while(consumed < length_) {
            u64 next = idx == capacity_ - 1 ? 0 : idx + 1;
            if(found) {
                if constexpr(Trivially_Movable<T>) {
                    Libc::memcpy(&data_[next], &data_[idx], sizeof(T));
                } else {
                    static_assert(Move_Constructable<T>);
                    new(&data_[next]) T{rpp::move(data_[idx])};
                }
            } else if(data_[idx] == value) {
                if constexpr(Must_Destruct<T>) {
                    data_[idx].~T();
                }
                found = true;
            }
            idx = next;
            consumed++;
        }
        if(found) {
            length_--;
            last_ = last_ ? last_ - 1 : capacity_ - 1;
        }
        return found;
    }

    template<bool is_const>
    struct Iterator {
        using Q = If<is_const, const Queue, Queue>;

        Iterator operator++() noexcept {
            count_++;
            return *this;
        }
        Iterator operator++(int) noexcept {
            Iterator prev{queue_, count_};
            count_++;
            return prev;
        }

        [[nodiscard]] T& operator*() const noexcept
            requires(!is_const)
        {
            return queue_.data_[queue_.idx_at(count_)];
        }
        [[nodiscard]] const T& operator*() const noexcept {
            return queue_.data_[queue_.idx_at(count_)];
        }

        [[nodiscard]] T* operator->() const noexcept
            requires(!is_const)
        {
            return &queue_.data_[queue_.idx_at(count_)];
        }
        [[nodiscard]] const T* operator->() const noexcept {
            return &queue_.data_[queue_.idx_at(count_)];
        }

        [[nodiscard]] bool operator==(const Iterator& rhs) const noexcept {
            return &queue_ == &rhs.queue_ && count_ == rhs.count_;
        }

    private:
        Iterator(Q& queue, u64 count) noexcept : queue_(queue), count_(count) {
        }

        Q& queue_;
        u64 count_ = 0;

        friend struct Queue;
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    [[nodiscard]] iterator begin() noexcept {
        return iterator{*this, 0};
    }
    [[nodiscard]] const_iterator begin() const noexcept {
        return const_iterator{*this, 0};
    }

    [[nodiscard]] iterator end() noexcept {
        return iterator{*this, length_};
    }
    [[nodiscard]] const_iterator end() const noexcept {
        return const_iterator{*this, length_};
    }

private:
    [[nodiscard]] u64 start_idx() const noexcept {
        u64 idx = last_ - length_;
        return idx >= capacity_ ? idx + capacity_ : idx;
    }
    [[nodiscard]] u64 end_idx() const noexcept {
        if(capacity_ == 0) return 0;
        return last_ == 0 ? capacity_ - 1 : last_ - 1;
    }
    [[nodiscard]] u64 idx_at(u64 i) const noexcept {
        u64 idx = last_ - length_ + i;
        return idx >= capacity_ ? idx + capacity_ : idx;
    }

    T* data_ = null;
    u64 length_ = 0;
    u64 last_ = 0;
    u64 capacity_ = 0;

    friend struct Reflect::Refl<Queue>;
    template<bool>
    friend struct Iterator;
};

template<typename T, Allocator A>
RPP_TEMPLATE_RECORD(Queue, RPP_PACK(T, A), RPP_FIELD(data_), RPP_FIELD(length_), RPP_FIELD(last_),
                    RPP_FIELD(capacity_));

namespace Format {

template<Reflectable T, Allocator A>
struct Measure<Queue<T, A>> {
    [[nodiscard]] static u64 measure(const Queue<T, A>& queue) noexcept {
        u64 n = 0;
        u64 length = 7;
        for(const T& item : queue) {
            length += Measure<T>::measure(item);
            if(n + 1 < queue.length()) length += 2;
            n++;
        }
        return length;
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Queue<T, A>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Queue<T, A>& queue) noexcept {
        idx = output.write(idx, "Queue["_v);
        u64 n = 0;
        for(const T& item : queue) {
            idx = Write<O, T>::write(output, idx, item);
            if(n + 1 < queue.length()) idx = output.write(idx, ", "_v);
            n++;
        }
        return output.write(idx, ']');
    }
};

} // namespace Format

} // namespace rpp
