
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T>
struct Ref {

    constexpr Ref() noexcept = default;
    constexpr explicit Ref(T& value) noexcept : value_(&value) {
    }
    constexpr ~Ref() noexcept {
        value_ = null;
    }

    constexpr Ref(const Ref& src) noexcept = default;
    constexpr Ref& operator=(const Ref&) noexcept = default;

    constexpr Ref(Ref&& src) noexcept : value_(src.value_) {
        src.value_ = null;
    }
    constexpr Ref& operator=(Ref&& src) noexcept {
        value_ = src.value_;
        src.value_ = null;
        return *this;
    }

    [[nodiscard]] constexpr T& operator*() noexcept;
    [[nodiscard]] constexpr const T& operator*() const noexcept;

    [[nodiscard]] constexpr T* operator->() noexcept;
    [[nodiscard]] constexpr const T* operator->() const noexcept;

    [[nodiscard]] constexpr bool ok() const noexcept {
        return value_ != null;
    }

private:
    T* value_ = null;
    friend struct Reflect::Refl<Ref<T>>;
};

template<typename T>
RPP_TEMPLATE_RECORD(Ref, T, RPP_FIELD(value_));

} // namespace rpp
