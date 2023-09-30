
#pragma once

namespace rpp {

template<typename T>
struct Storage {

    Storage() = default;

    template<typename... Args>
        requires Constructable<T, Args...>
    explicit Storage(Args&&... args) {
        new(value_) T{std::forward<Args>(args)...};
    }

    ~Storage() = default;

    Storage(const Storage& src)
        requires Trivial<T>
    = default;
    Storage& operator=(const Storage&)
        requires Trivial<T>
    = default;

    Storage(Storage&& src)
        requires Trivial<T>
    = default;
    Storage& operator=(Storage&&)
        requires Trivial<T>
    = default;

    template<typename... Args>
        requires Constructable<T, Args...>
    void construct(Args&&... args) {
        new(value_) T{std::forward<Args>(args)...};
    }

    void destruct() {
        if constexpr(Must_Destruct<T>) {
            reinterpret_cast<T*>(value_)->~T();
        }
    }

    T& operator*() {
        return *reinterpret_cast<T*>(value_);
    }
    const T& operator*() const {
        return *reinterpret_cast<const T*>(value_);
    }

    T* operator->() {
        return reinterpret_cast<T*>(value_);
    }
    const T* operator->() const {
        return reinterpret_cast<const T*>(value_);
    }

protected:
    alignas(alignof(T)) u8 value_[sizeof(T)] = {};

    friend struct Reflect<Storage>;
};

template<typename S>
struct Reflect<Storage<S>> {
    using T = Storage<S>;
    static constexpr Literal name = "Storage";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(value_)>;
};

} // namespace rpp