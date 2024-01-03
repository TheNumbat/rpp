
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<Allocator A>
void String<A>::set_length(u64 length) noexcept {
    assert(length <= capacity_);
    length_ = length;
}

template<Allocator A>
[[nodiscard]] const u8& String<A>::operator[](u64 idx) const noexcept {
    assert(idx < length_);
    return data_[idx];
}

template<Allocator A>
[[nodiscard]] u8& String<A>::operator[](u64 idx) noexcept {
    assert(idx < length_);
    return data_[idx];
}

template<Allocator A>
[[nodiscard]] String_View String<A>::sub(u64 start, u64 end) const noexcept {
    assert(start <= end);
    assert(end <= length_);
    return String_View{data_ + start, end - start};
}

template<Allocator A>
[[nodiscard]] String<A> String_View::terminate() const noexcept {
    String<A> ret{length_ + 1};
    ret.set_length(length_ + 1);
    Libc::memcpy(ret.data(), data_, length_);
    ret[length_] = '\0';
    return ret;
}

template<Allocator SA>
template<Allocator RA>
[[nodiscard]] String<RA> String<SA>::terminate() const noexcept {
    String<RA> ret{length_ + 1};
    ret.set_length(length_ + 1);
    Libc::memcpy(ret.data(), data_, length_);
    ret[length_] = '\0';
    return ret;
}

[[nodiscard]] constexpr String_View String_View::sub(u64 start, u64 end) const noexcept {
    assert(start <= end);
    assert(end <= length_);
    return String_View{data_ + start, end - start};
}

[[nodiscard]] constexpr const u8& String_View::operator[](u64 idx) const noexcept {
    assert(idx < length_);
    return data_[idx];
}

[[nodiscard]] constexpr String_View String_View::file_suffix() const noexcept {

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

[[nodiscard]] constexpr String_View String_View::file_extension() const noexcept {

    if(length_ == 0) return String_View{};

    u64 i = length_ - 1;
    u64 offset = 0;

    for(; i != 0; i--) {
        if(data_[i] == '.' || data_[i] == '\\' || data_[i] == '/') {
            offset = 1;
            break;
        }
    }
    return String_View{data_ + i + offset, length_ - i - offset};
}

[[nodiscard]] constexpr String_View String_View::remove_file_suffix() const noexcept {

    if(length_ == 0) return String_View{};

    bool found = 0;
    u64 i = length_ - 1;

    for(; i != 0; i--) {
        if(data_[i] == '\\' || data_[i] == '/') {
            found = 1;
            break;
        }
    }
    return String_View{data_, i + found};
}

template<Allocator A>
[[nodiscard]] u64 String<A>::write(u64 i, char c) noexcept {
    assert(i < length_);
    data_[i] = c;
    return i + 1;
}

template<Allocator A>
template<Allocator B>
[[nodiscard]] u64 String<A>::write(u64 i, const String<B>& text) noexcept {
    assert(i + text.length() <= length_);
    Libc::memcpy(data_ + i, text.data(), text.length());
    return i + text.length();
}

template<Allocator A>
[[nodiscard]] u64 String<A>::write(u64 i, String_View text) noexcept {
    assert(i + text.length() <= length_);
    Libc::memcpy(data_ + i, text.data(), text.length());
    return i + text.length();
}

template<Allocator A>
[[nodiscard]] String<A> String_View::append(String_View next) const noexcept {
    String<A> ret{length_ + next.length()};
    ret.set_length(length_ + next.length());
    Libc::memcpy(ret.data(), data_, length_);
    Libc::memcpy(ret.data() + length_, next.data(), next.length());
    return ret;
}

template<Allocator A>
template<Allocator RA, Allocator B>
[[nodiscard]] String<RA> String<A>::append(const String<B>& next) const noexcept {
    String<RA> ret{length_ + next.length()};
    ret.set_length(length_ + next.length());
    Libc::memcpy(ret.data(), data_, length_);
    Libc::memcpy(ret.data() + length_, next.data(), next.length());
    return ret;
}

namespace Format {

template<>
struct Measure<String_View> {
    [[nodiscard]] constexpr static u64 measure(String_View string) noexcept {
        return string.length();
    }
};
template<Allocator A>
struct Measure<String<A>> {
    [[nodiscard]] static u64 measure(const String<A>& string) noexcept {
        return string.length();
    }
};

template<Allocator O>
struct Write<O, String_View> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, String_View value) noexcept {
        return output.write(idx, value);
    }
};
template<Allocator O, Allocator A>
struct Write<O, String<A>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const String<A>& value) noexcept {
        return output.write(idx, value);
    }
};

} // namespace Format

} // namespace rpp