
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T>
struct Ref {

    constexpr Ref() = default;
    constexpr explicit Ref(T& value) : value_(&value) {
    }
    constexpr ~Ref() {
        value_ = null;
    }

    constexpr Ref(const Ref& src) = default;
    constexpr Ref& operator=(const Ref&) = default;

    constexpr Ref(Ref&& src) : value_(src.value_) {
        src.value_ = null;
    }
    constexpr Ref& operator=(Ref&& src) {
        value_ = src.value_;
        src.value_ = null;
        return *this;
    }

    constexpr T& operator*();
    constexpr const T& operator*() const;

    constexpr T* operator->();
    constexpr const T* operator->() const;

    constexpr operator bool() const {
        return value_ != null;
    }

private:
    T* value_ = null;
    friend struct Reflect::Refl<Ref<T>>;
};

template<typename T>
RPP_TEMPLATE_RECORD(Ref, T, RPP_FIELD(value_));

} // namespace rpp
