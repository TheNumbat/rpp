
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T>
struct Ref {

    Ref() = default;
    explicit Ref(T& value) : value_(&value) {
    }
    ~Ref() {
        value_ = null;
    }

    Ref(const Ref& src) = default;
    Ref& operator=(const Ref&) = default;

    Ref(Ref&& src) : value_(src.value_) {
        src.value_ = null;
    }
    Ref& operator=(Ref&& src) {
        value_ = src.value_;
        src.value_ = null;
        return *this;
    }

    T& operator*();
    const T& operator*() const;

    T* operator->();
    const T* operator->() const;

    operator bool() const {
        return value_ != null;
    }

private:
    T* value_ = null;
    friend struct Reflect<Ref<T>>;
};

namespace detail {

template<typename R>
struct Reflect<Ref<R>> {
    using T = Ref<R>;
    static constexpr Literal name = "Ref";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(value_)>;
};

} // namespace detail

} // namespace rpp
