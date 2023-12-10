
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<Movable T, Allocator A = Mdefault>
struct Queue {

    Queue() = default;
    explicit Queue(u64 capacity) {
        data_ = reinterpret_cast<T*>(A::alloc(capacity * sizeof(T)));
        length_ = 0;
        last_ = 0;
        capacity_ = capacity;
    }

    template<typename... Ss>
        requires All<T, Ss...> && Move_Constructable<T>
    explicit Queue(Ss&&... init) {
        reserve(sizeof...(Ss));
        (push(std::move(init)), ...);
    }

    Queue(const Queue& src) = delete;
    Queue& operator=(const Queue& src) = delete;

    Queue(Queue&& src) {
        data_ = src.data_;
        length_ = src.length_;
        last_ = src.last_;
        capacity_ = src.capacity_;
        src.data_ = null;
        src.length_ = 0;
        src.last_ = 0;
        src.capacity_ = 0;
    }
    Queue& operator=(Queue&& src) {
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

    ~Queue() {
        clear();
        A::free(data_);
        data_ = null;
        capacity_ = 0;
    }

    template<Allocator B = A>
    Queue<T, B> clone() const
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

    void grow() {
        u64 new_capacity = capacity_ ? 2 * capacity_ : 8;
        reserve(new_capacity);
    }

    void reserve(u64 new_capacity) {
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
                    new(&new_data[i]) T{std::move(start[i])};
                }
            } else {
                u64 first = length_ - last_;
                for(u64 i = 0; i < first; i++) {
                    new(&new_data[i]) T{std::move(start[i + capacity_])};
                }
                for(u64 i = 0; i < last_; i++) {
                    new(&new_data[first + i]) T{std::move(data_[i])};
                }
            }
        }

        A::free(data_);
        last_ = length_;
        capacity_ = new_capacity;
        data_ = new_data;
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

        new(&data_[last_]) T{std::move(value)};
        T& ret = data_[last_];

        length_++;
        last_ = last_ == capacity_ - 1 ? 0 : last_ + 1;
        return ret;
    }

    template<typename... Args>
    T& emplace(Args&&... args)
        requires Constructable<T, Args...>
    {
        if(full()) grow();

        new(&data_[last_]) T{std::forward<Args>(args)...};
        T& ret = data_[last_];

        length_++;
        last_ = last_ == capacity_ - 1 ? 0 : last_ + 1;
        return ret;
    }

    void pop() {
        assert(length_ > 0);
        u64 idx = length_ <= last_ ? last_ - length_ : last_ - length_ + capacity_;
        length_--;
        data_[idx].~T();
    }

    void clear() {
        if constexpr(Must_Destruct<T>) {
            for(T& value : *this) {
                value.~T();
            }
        }
        length_ = 0;
        last_ = 0;
    }

    T& front() {
        assert(!empty());
        return data_[start_idx()];
    }
    const T& front() const {
        assert(!empty());
        return data_[start_idx()];
    }

    T& back() {
        assert(!empty());
        return data_[end_idx()];
    }
    const T& back() const {
        assert(!empty());
        return data_[end_idx()];
    }

    T& penultimate() {
        assert(length_ > 1);
        u64 idx = last_ == 0 ? capacity_ - 2 : last_ == 1 ? capacity_ - 1 : last_ - 2;
        return data_[idx];
    }
    const T& penultimate() const {
        assert(length_ > 1);
        u64 idx = last_ == 0 ? capacity_ - 2 : last_ == 1 ? capacity_ - 1 : last_ - 2;
        return data_[idx];
    }

    bool empty() const {
        return length_ == 0;
    }
    bool full() const {
        return length_ == capacity_;
    }
    u64 length() const {
        return length_;
    }

    T& operator[](u64 idx) {
        assert(idx < length_);
        u64 i = idx + start_idx();
        i = i >= capacity_ ? i - capacity_ : i;
        return data_[i];
    }
    const T& operator[](u64 idx) const {
        assert(idx < length_);
        u64 i = idx + start_idx();
        i = i >= capacity_ ? i - capacity_ : i;
        return data_[i];
    }

    bool erase_first(const T& value)
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
                    new(&data_[next]) T{std::move(data_[idx])};
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

    template<bool const_>
    struct Iterator {
        using Q = typename If<const_, const Queue, Queue>::type;

        Iterator operator++() {
            count_++;
            return *this;
        }
        Iterator operator++(int) {
            Iterator prev{queue_, count_};
            count_++;
            return prev;
        }

        T& operator*() const
            requires(!const_)
        {
            return queue_.data_[queue_.idx_at(count_)];
        }
        const T& operator*() const {
            return queue_.data_[queue_.idx_at(count_)];
        }

        T* operator->() const
            requires(!const_)
        {
            return &queue_.data_[queue_.idx_at(count_)];
        }
        const T* operator->() const {
            return &queue_.data_[queue_.idx_at(count_)];
        }

        bool operator==(const Iterator& rhs) const {
            return &queue_ == &rhs.queue_ && count_ == rhs.count_;
        }

    private:
        Iterator(Q& queue, u64 count) : queue_(queue), count_(count) {
        }

        Q& queue_;
        u64 count_ = 0;

        friend struct Queue;
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    iterator begin() {
        return iterator{*this, 0};
    }
    const_iterator begin() const {
        return const_iterator{*this, 0};
    }

    iterator end() {
        return iterator{*this, length_};
    }
    const_iterator end() const {
        return const_iterator{*this, length_};
    }

private:
    u64 start_idx() const {
        u64 idx = last_ - length_;
        return idx >= capacity_ ? idx + capacity_ : idx;
    }
    u64 end_idx() const {
        if(capacity_ == 0) return 0;
        return last_ == 0 ? capacity_ - 1 : last_ - 1;
    }
    u64 idx_at(u64 i) const {
        u64 idx = last_ - length_ + i;
        return idx >= capacity_ ? idx + capacity_ : idx;
    }

    T* data_ = null;
    u64 length_ = 0;
    u64 last_ = 0;
    u64 capacity_ = 0;

    friend struct Reflect<Queue>;
    template<bool>
    friend struct Iterator;
};

template<typename Q, Allocator A>
struct rpp::detail::Reflect<Queue<Q, A>> {
    using T = Queue<Q, A>;
    static constexpr Literal name = "Queue";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_), FIELD(length_), FIELD(last_), FIELD(capacity_)>;
};

namespace Format {

template<Reflectable T, Allocator A>
struct Measure<Queue<T, A>> {
    static u64 measure(const Queue<T, A>& queue) {
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
    static u64 write(String<O>& output, u64 idx, const Queue<T, A>& queue) {
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
