
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T>
struct Storage {

    Storage() noexcept = default;

    template<typename... Args>
        requires Constructable<T, Args...>
    explicit Storage(Args&&... args) noexcept {
        new(value_) T{forward<Args>(args)...};
    }

    ~Storage() noexcept = default;

    Storage(const Storage& src) noexcept
        requires Trivially_Copyable<T>
    = default;
    Storage& operator=(const Storage&) noexcept
        requires Trivially_Copyable<T>
    = default;

    Storage(Storage&& src) noexcept
        requires Trivially_Movable<T>
    = default;
    Storage& operator=(Storage&&) noexcept
        requires Trivially_Movable<T>
    = default;

    template<typename... Args>
        requires Constructable<T, Args...>
    void construct(Args&&... args) noexcept {
        new(value_) T{forward<Args>(args)...};
    }

    void destruct() noexcept {
        if constexpr(Must_Destruct<T>) {
            reinterpret_cast<T*>(value_)->~T();
        }
    }

    [[nodiscard]] T& operator*() noexcept {
        return *reinterpret_cast<T*>(value_);
    }
    [[nodiscard]] const T& operator*() const noexcept {
        return *reinterpret_cast<const T*>(value_);
    }

    [[nodiscard]] T* operator->() noexcept {
        return reinterpret_cast<T*>(value_);
    }
    [[nodiscard]] const T* operator->() const noexcept {
        return reinterpret_cast<const T*>(value_);
    }

    [[nodiscard]] const u8* data() const noexcept {
        return value_;
    }
    [[nodiscard]] u8* data() noexcept {
        return value_;
    }

protected:
    alignas(alignof(T)) u8 value_[sizeof(T)];

    friend struct Reflect::Refl<Storage>;
};

template<typename T>
RPP_TEMPLATE_RECORD(Storage, T, RPP_FIELD(value_));

namespace Format {

template<Reflectable T>
struct Measure<Storage<T>> {
    [[nodiscard]] static u64 measure(const Storage<T>&) noexcept {
        return 9 + String_View{Reflect::Refl<T>::name}.length();
    }
};
template<Allocator O, Reflectable T>
struct Write<O, Storage<T>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Storage<T>&) noexcept {
        idx = output.write(idx, "Storage<"_v);
        idx = output.write(idx, String_View{Reflect::Refl<T>::name});
        return output.write(idx, '>');
    }
};

} // namespace Format

} // namespace rpp