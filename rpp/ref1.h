
#pragma once

namespace rpp {

template<typename T>
T& Ref<T>::operator*() {
    assert(value_);
    return *value_;
}

template<typename T>
const T& Ref<T>::operator*() const {
    assert(value_);
    return *value_;
}

template<typename T>
T* Ref<T>::operator->() {
    assert(value_);
    return value_;
}

template<typename T>
const T* Ref<T>::operator->() const {
    assert(value_);
    return value_;
}

} // namespace rpp
