
#pragma once

namespace rpp {

template<Allocator A>
const u8& String<A>::operator[](u64 idx) const {
    assert(idx < length_);
    return data_[idx];
}

template<Allocator A>
u8& String<A>::operator[](u64 idx) {
    assert(idx < length_);
    return data_[idx];
}

inline const u8& String_View::operator[](u64 idx) const {
    assert(idx < length_);
    return data_[idx];
}

inline String_View String_View::file_suffix() const {

    if(length_ == 0) return String_View{};

    u64 i = length_ - 1;
    u64 offset = 0;

    for(; i != 0; i--) {
        if(data_[i] == '\\' || data_[i] == '/') {
            offset = 1;
            break;
        }
    }
    return String_View{data_ + i + offset, length_ - i - offset};
}

} // namespace rpp