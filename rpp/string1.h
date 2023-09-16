
#pragma once

namespace rpp {

template<Allocator A>
void String<A>::set_length(u64 length) {
    assert(length <= capacity_);
    length_ = length;
}

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

template<Allocator A>
u64 String<A>::write(u64 i, char c) {
    assert(i < length_);
    data_[i] = c;
    return i + 1;
}

template<Allocator A>
template<Allocator B>
u64 String<A>::write(u64 i, const String<B>& text) {
    assert(i + text.length() <= length_);
    std::memcpy(data_ + i, text.data(), text.length());
    return i + text.length();
}

template<Allocator A>
u64 String<A>::write(u64 i, const String_View& text) {
    assert(i + text.length() <= length_);
    std::memcpy(data_ + i, text.data(), text.length());
    return i + text.length();
}

} // namespace rpp