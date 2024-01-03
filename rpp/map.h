
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T>
concept Key = Equality<T> && Hashable<T> && Movable<T>;

namespace detail {

template<Key K, Movable V>
struct Map_Slot {
    constexpr Map_Slot() = default;

    constexpr explicit Map_Slot(K&& key, V&& value) noexcept : hash(hash_nonzero(key)) {
        data.construct(move(key), move(value));
    }
    constexpr ~Map_Slot() noexcept {
        if constexpr(Must_Destruct<Pair<K, V>>) {
            if(hash != EMPTY) data.destruct();
        }
        hash = EMPTY;
    }

    constexpr Map_Slot(const Map_Slot&) noexcept = delete;
    constexpr Map_Slot& operator=(const Map_Slot&) noexcept = delete;

    constexpr Map_Slot(Map_Slot&& src) noexcept {
        hash = src.hash;
        src.hash = EMPTY;
        if(hash != EMPTY) data.construct(move(*src.data));
    }
    constexpr Map_Slot& operator=(Map_Slot&& src) noexcept {
        this->~Map_Slot();
        hash = src.hash;
        src.hash = EMPTY;
        if(hash != EMPTY) data.construct(move(*src.data));
        return *this;
    }

    [[nodiscard]] constexpr Map_Slot clone() const noexcept
        requires((Clone<K> || Copy_Constructable<K>) && (Clone<V> || Copy_Constructable<V>))
    {
        if(hash == EMPTY) return Map_Slot{};
        if constexpr(Clone<K> && Clone<V>) {
            return Map_Slot{data->first.clone(), data->second.clone()};
        } else if constexpr(Clone<K> && Copy_Constructable<V>) {
            return Map_Slot{data->first.clone(), V{data->second}};
        } else if constexpr(Copy_Constructable<K> && Clone<V>) {
            return Map_Slot{K{data->first}, data->second.clone()};
        } else {
            static_assert(Copy_Constructable<K> && Copy_Constructable<V>);
            return Map_Slot{K{data->first}, V{data->second}};
        }
    }

    constexpr static u64 EMPTY = 0;
    u64 hash = EMPTY;
    Storage<Pair<K, V>> data;
};

} // namespace detail

template<Key K, Movable V, Allocator A = Mdefault>
struct Map {
    using Slot = detail::Map_Slot<K, V>;

    Map() noexcept = default;

    explicit Map(u64 capacity) noexcept {
        capacity_ = Math::next_pow2(capacity);
        shift_ = Math::ctlz(capacity_) + 1;
        usable_ = (capacity_ / 4) * 3;
        length_ = 0;
        data_ = reinterpret_cast<Slot*>(A::alloc(capacity_ * sizeof(Slot)));
        Libc::memset(data_, 0, capacity_ * sizeof(Slot));
    }

    template<typename... Ss>
        requires All_Are<Pair<K, V>, Ss...> && Move_Constructable<Pair<K, V>>
    explicit Map(Ss&&... init) noexcept {
        (insert(move(init.first), move(init.second)), ...);
    }

    Map(const Map& src) noexcept = delete;
    Map& operator=(const Map& src) noexcept = delete;

    Map(Map&& src) noexcept {
        data_ = src.data_;
        capacity_ = src.capacity_;
        length_ = src.length_;
        usable_ = src.usable_;
        shift_ = src.shift_;
        src.data_ = null;
        src.capacity_ = 0;
        src.length_ = 0;
        src.usable_ = 0;
        src.shift_ = 0;
    }
    Map& operator=(Map&& src) noexcept {
        this->~Map();
        data_ = src.data_;
        capacity_ = src.capacity_;
        length_ = src.length_;
        usable_ = src.usable_;
        shift_ = src.shift_;
        src.data_ = null;
        src.capacity_ = 0;
        src.length_ = 0;
        src.usable_ = 0;
        src.shift_ = 0;
        return *this;
    }

    ~Map() noexcept {
        if constexpr(Must_Destruct<Pair<K, V>>) {
            for(u64 i = 0; i < capacity_; i++) {
                data_[i].~Slot();
            }
        }
        A::free(data_);
        data_ = null;
        capacity_ = 0;
        length_ = 0;
        usable_ = 0;
        shift_ = 0;
    }

    template<Allocator B = A>
    [[nodiscard]] Map<K, V, B> clone() const noexcept
        requires((Clone<K> || Copy_Constructable<K>) && (Clone<V> || Copy_Constructable<V>))
    {
        Map<K, V, B> ret(capacity_);
        ret.length_ = length_;
        if constexpr(Trivially_Copyable<K> && Trivially_Copyable<V>) {
            Libc::memcpy(ret.data_, data_, capacity_ * sizeof(Slot));
        } else {
            for(u64 i = 0; i < capacity_; i++) {
                if(data_[i].hash != Slot::EMPTY) new(&ret.data_[i]) Slot{data_[i].clone()};
            }
        }
        return ret;
    }

    void reserve(u64 new_capacity) noexcept {
        if(new_capacity <= capacity_) return;

        Slot* old_data = data_;
        u64 old_capacity = capacity_;

        capacity_ = new_capacity;
        data_ = reinterpret_cast<Slot*>(A::alloc(capacity_ * sizeof(Slot)));
        Libc::memset(data_, 0, capacity_ * sizeof(Slot));
        usable_ = (capacity_ / 4) * 3;
        shift_ = Math::ctlz(capacity_) + 1;

        for(u64 i = 0; i < old_capacity; i++) {
            if(old_data[i].hash != Slot::EMPTY) static_cast<void>(insert_slot(move(old_data[i])));
        }
        A::free(old_data);
    }

    void grow() noexcept {
        u64 new_capacity = capacity_ ? 2 * capacity_ : 32;
        reserve(new_capacity);
    }

    void clear() noexcept {
        if constexpr(Must_Destruct<Pair<K, V>>) {
            for(u64 i = 0; i < capacity_; i++) {
                data_[i].~Slot();
            }
        } else {
            Libc::memset(data_, Slot::EMPTY, capacity_ * sizeof(Slot));
        }
        length_ = 0;
    }

    [[nodiscard]] bool empty() const noexcept {
        return length_ == 0;
    }
    [[nodiscard]] bool full() const noexcept {
        return length_ == usable_;
    }
    [[nodiscard]] u64 length() const noexcept {
        return length_;
    }

    V& insert(const K& key, const V& value) noexcept
        requires Copy_Constructable<K> && Copy_Constructable<V>
    {
        return insert(K{key}, V{value});
    }

    V& insert(K&& key, const V& value) noexcept
        requires Copy_Constructable<V>
    {
        return insert(move(key), V{value});
    }

    V& insert(const K& key, V&& value) noexcept
        requires Copy_Constructable<K>
    {
        return insert(K{key}, move(value));
    }

    V& insert(K&& key, V&& value) noexcept {
        if(full()) grow();
        Slot slot{move(key), move(value)};
        Slot& placed = insert_slot(move(slot));
        length_ += 1;
        return placed.data->second;
    }

    template<typename... Args>
        requires Constructable<V, Args...>
    V& emplace(K&& key, Args&&... args) noexcept {
        if(full()) grow();
        Slot slot{move(key), V{forward<Args>(args)...}};
        Slot& placed = insert_slot(move(slot));
        length_ += 1;
        return placed.data->second;
    }

    [[nodiscard]] Opt<Ref<V>> try_get(const K& key) noexcept {
        if(empty()) return {};
        if(auto idx = try_get_<K>(key)) {
            return Opt{Ref{data_[*idx].data->second}};
        }
        return {};
    }

    [[nodiscard]] Opt<Ref<const V>> try_get(const K& key) const noexcept {
        if(empty()) return {};
        if(auto idx = try_get_<K>(key)) {
            return Opt{Ref<const V>{data_[*idx].data->second}};
        }
        return {};
    }

    [[nodiscard]] bool try_erase(const K& key) noexcept {
        if(empty()) return false;
        u64 hash = hash_nonzero(key);
        u64 idx = hash >> shift_;
        u64 dist = 0;
        for(;;) {
            u64 k = data_[idx].hash;
            if(k == hash && data_[idx].data->first == key) {
                data_[idx].~Slot();
                fix_up(idx);
                length_ -= 1;
                return true;
            }
            u64 kidx = k >> shift_;
            u64 kdist = kidx <= idx ? idx - kidx : capacity_ + idx - kidx;
            if(kdist < dist) return false;
            dist++;
            if(++idx == capacity_) idx = 0;
        }
    }

    [[nodiscard]] bool contains(String_View key) const noexcept
        requires(Any_String<K>)
    {
        if(empty()) return false;
        return try_get_<String_View>(key);
    }

    [[nodiscard]] Opt<Ref<V>> try_get(String_View key) noexcept
        requires(Any_String<K>)
    {
        if(empty()) return {};
        if(auto idx = try_get_<String_View>(key)) {
            return Opt<Ref<V>>{data_[*idx].data->second};
        }
        return {};
    }

    [[nodiscard]] V& get(String_View key) noexcept
        requires(Any_String<K>)
    {
        if(auto idx = try_get_<String_View>(key)) {
            return data_[*idx].data->second;
        }
        die("Failed to find key %!", key);
    }

    [[nodiscard]] bool contains(const K& key) const noexcept {
        return try_get(key);
    }

    [[nodiscard]] V& get(const K& key) noexcept {
        Opt<Ref<V>> value = try_get(key);
        if(!value) die("Failed to find key %!", key);
        return **value;
    }

    [[nodiscard]] const V& get(const K& key) const noexcept {
        Opt<Ref<const V>> value = try_get(key);
        if(!value) die("Failed to find key %!", key);
        return **value;
    }

    void erase(const K& key) noexcept {
        if(!try_erase(key)) die("Failed to erase key %!", key);
    }

    [[nodiscard]] V& get_or_insert(const K& key) noexcept
        requires Copy_Constructable<K> && Default_Constructable<V>
    {
        Opt<Ref<V>> entry = try_get(key);
        if(entry) {
            return **entry;
        }
        return insert(K{key}, V{});
    }

    [[nodiscard]] V& get_or_insert(K&& key) noexcept
        requires Default_Constructable<V>
    {
        Opt<Ref<V>> entry = try_get(key);
        if(entry) {
            return **entry;
        }
        return insert(move(key), V{});
    }

    template<bool is_const>
    struct Iterator {
        using M = If<is_const, const Map, Map>;

        Iterator operator++(int) noexcept {
            Iterator i = *this;
            count_++;
            skip();
            return i;
        }
        Iterator operator++() noexcept {
            count_++;
            skip();
            return *this;
        }

        [[nodiscard]] Pair<const K, V>& operator*() const noexcept
            requires(!is_const)
        {
            return reinterpret_cast<Pair<const K, V>&>(*map_.data_[count_].data);
        }
        [[nodiscard]] const Pair<K, V>& operator*() const noexcept {
            return *map_.data_[count_].data;
        }

        [[nodiscard]] Pair<const K, V>* operator->() const noexcept
            requires(!is_const)
        {
            return reinterpret_cast<Pair<const K, V>*>(&*map_.data_[count_].data);
        }
        [[nodiscard]] const Pair<K, V>* operator->() const noexcept {
            return &*map_.data_[count_].data;
        }

        [[nodiscard]] bool operator==(const Iterator& rhs) const noexcept {
            return &map_ == &rhs.map_ && count_ == rhs.count_;
        }

    private:
        void skip() noexcept {
            while(count_ < map_.capacity_ && map_.data_[count_].hash == Slot::EMPTY) count_++;
        }
        Iterator(M& map, u64 count) noexcept : map_(map), count_(count) {
            skip();
        }
        M& map_;
        u64 count_ = 0;

        friend struct Map;
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    [[nodiscard]] const_iterator begin() const noexcept {
        return const_iterator(*this, 0);
    }
    [[nodiscard]] const_iterator end() const noexcept {
        return const_iterator(*this, capacity_);
    }
    [[nodiscard]] iterator begin() noexcept {
        return iterator(*this, 0);
    }
    [[nodiscard]] iterator end() noexcept {
        return iterator(*this, capacity_);
    }

private:
    [[nodiscard]] Slot& insert_slot(Slot&& slot) noexcept {
        u64 idx = slot.hash >> shift_;
        Slot* placement = null;
        u64 dist = 0;
        for(;;) {
            u64 hash = data_[idx].hash;
            if(hash == Slot::EMPTY) {
                data_[idx] = move(slot);
                return placement ? *placement : data_[idx];
            }
            if(hash == slot.hash && data_[idx].data->first == slot.data->first) {
                data_[idx] = move(slot);
                return data_[idx];
            }
            u64 hashidx = hash >> shift_;
            u64 hashdist = hashidx <= idx ? idx - hashidx : capacity_ + idx - hashidx;
            if(hashdist < dist) {
                swap(data_[idx], slot);
                placement = placement ? placement : &data_[idx];
                dist = hashdist;
            }
            dist++;
            if(++idx == capacity_) idx = 0;
        }
    }

    void fix_up(u64 idx) noexcept {
        for(;;) {
            u64 next = idx == capacity_ - 1 ? 0 : idx + 1;
            u64 nexthash_ = data_[next].hash;
            if(nexthash_ == Slot::EMPTY) return;
            u64 next_ideal = nexthash_ >> shift_;
            if(next == next_ideal) return;
            data_[idx] = move(data_[next]);
            idx = next;
        }
    }

    template<Hashable K2>
    [[nodiscard]] Opt<u64> try_get_(const K2& key) const noexcept {
        u64 hash = hash_nonzero(key);
        u64 idx = hash >> shift_;
        u64 dist = 0;
        for(;;) {
            u64 k = data_[idx].hash;
            if(k == Slot::EMPTY) return {};
            if(k == hash && data_[idx].data->first == key) {
                return Opt<u64>{idx};
            }
            u64 kidx = k >> shift_;
            u64 kdist = kidx <= idx ? idx - kidx : capacity_ + idx - kidx;
            if(kdist < dist) return {};
            dist++;
            if(++idx == capacity_) idx = 0;
        }
    }

    Slot* data_ = null;
    u64 capacity_ = 0;
    u64 length_ = 0;
    u64 usable_ = 0;
    u64 shift_ = 0;

    friend struct Reflect::Refl<Map>;
    template<bool>
    friend struct Iterator;
};

template<Key K, Movable V>
RPP_NAMED_TEMPLATE_RECORD(::rpp::detail::Map_Slot, "Slot", RPP_PACK(K, V), RPP_FIELD(hash),
                          RPP_FIELD(data_));

template<Key K, Movable V, Allocator A>
RPP_TEMPLATE_RECORD(Map, RPP_PACK(K, V, A), RPP_FIELD(data_), RPP_FIELD(capacity_),
                    RPP_FIELD(length_), RPP_FIELD(usable_), RPP_FIELD(shift_));

namespace Format {

template<Reflectable K, Reflectable V, Allocator A>
struct Measure<Map<K, V, A>> {
    [[nodiscard]] static u64 measure(const Map<K, V, A>& map) noexcept {
        u64 n = 0;
        u64 length = 5;
        for(const Pair<K, V>& item : map) {
            length += 5;
            length += Measure<K>::measure(item.first) + Measure<V>::measure(item.second);
            if(n + 1 < map.length()) length += 2;
            n++;
        }
        return length;
    }
};

template<Allocator O, Reflectable K, Reflectable V, Allocator A>
struct Write<O, Map<K, V, A>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Map<K, V, A>& map) noexcept {
        idx = output.write(idx, "Map["_v);
        u64 n = 0;
        for(const Pair<K, V>& item : map) {
            idx = output.write(idx, "{"_v);
            idx = Write<O, K>::write(output, idx, item.first);
            idx = output.write(idx, " : "_v);
            idx = Write<O, V>::write(output, idx, item.second);
            idx = output.write(idx, '}');
            if(n + 1 < map.length()) idx = output.write(idx, ", "_v);
            n++;
        }
        return output.write(idx, ']');
    }
};

} // namespace Format

} // namespace rpp
