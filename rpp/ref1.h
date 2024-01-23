
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T>
[[nodiscard]] constexpr T& Ref<T>::operator*() noexcept {
    assert(value_);
    return *value_;
}

template<typename T>
[[nodiscard]] constexpr const T& Ref<T>::operator*() const noexcept {
    assert(value_);
    return *value_;
}

template<typename T>
[[nodiscard]] constexpr T* Ref<T>::operator->() noexcept {
    assert(value_);
    return value_;
}

template<typename T>
[[nodiscard]] constexpr const T* Ref<T>::operator->() const noexcept {
    assert(value_);
    return value_;
}

namespace Format {

template<Reflectable T>
struct Measure<Ref<T>> {
    [[nodiscard]] static u64 measure(const Ref<T>& ref) noexcept {
        if(ref.ok()) return 5 + Measure<T>::measure(*ref);
        return 9;
    }
};
template<Allocator O, Reflectable T>
struct Write<O, Ref<T>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Ref<T>& ref) noexcept {
        if(!ref.ok()) return output.write(idx, "Ref{null}"_v);
        idx = output.write(idx, "Ref{"_v);
        idx = Write<O, T>::write(output, idx, *ref);
        return output.write(idx, '}');
    }
};

} // namespace Format

} // namespace rpp
