
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<Allocator A = Mdefault>
struct String;

struct String_View {

    constexpr String_View() noexcept = default;

    explicit String_View(const char* c_string) noexcept
        : data_(reinterpret_cast<const u8*>(c_string)), length_(Libc::strlen(c_string)) {
    }

    explicit String_View(const Literal& literal) noexcept
        : data_(reinterpret_cast<const u8*>(literal.c_string)) {
        while(length_ < Literal::max_len && literal.c_string[length_]) length_++;
    }

    constexpr explicit String_View(const u8* data, u64 length) noexcept
        : data_(data), length_(length) {
    }

    constexpr ~String_View() noexcept = default;

    constexpr String_View(const String_View& src) noexcept = default;
    constexpr String_View& operator=(const String_View& src) noexcept = default;

    constexpr String_View(String_View&& src) noexcept = default;
    constexpr String_View& operator=(String_View&& src) noexcept = default;

    [[nodiscard]] constexpr const u8& operator[](u64 idx) const noexcept;

    [[nodiscard]] constexpr String_View file_suffix() const noexcept;
    [[nodiscard]] constexpr String_View file_extension() const noexcept;
    [[nodiscard]] constexpr String_View remove_file_suffix() const noexcept;

    template<Allocator A = Mdefault>
    [[nodiscard]] String<A> string() const noexcept;

    [[nodiscard]] constexpr const u8* begin() const noexcept {
        return data_;
    }
    [[nodiscard]] constexpr const u8* end() const noexcept {
        return data_ + length_;
    }

    [[nodiscard]] constexpr const u8* data() const noexcept {
        return data_;
    }
    [[nodiscard]] constexpr u64 length() const noexcept {
        return length_;
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return length_ == 0;
    }

    template<Allocator A>
    [[nodiscard]] String<A> terminate() const noexcept;

    template<Allocator A>
    [[nodiscard]] String<A> append(String_View next) const noexcept;

    [[nodiscard]] constexpr String_View sub(u64 start, u64 end) const noexcept;

    [[nodiscard]] constexpr String_View clone() const noexcept {
        return String_View{data_, length_};
    }

private:
    const u8* data_ = null;
    u64 length_ = 0;

    friend struct Reflect::Refl<String_View>;
};

template<Allocator A>
struct String {

    String() noexcept = default;
    explicit String(u64 capacity) noexcept
        : data_(reinterpret_cast<u8*>(A::alloc(capacity))), length_(0), capacity_(capacity) {
    }

    ~String() noexcept {
        A::free(data_);
        data_ = null;
        capacity_ = 0;
        length_ = 0;
    }

    String(const String& src) noexcept = delete;
    String& operator=(const String& src) noexcept = delete;

    String(String&& src) noexcept
        : data_(src.data_), length_(src.length_), capacity_(src.capacity_) {
        src.data_ = null;
        src.length_ = 0;
        src.capacity_ = 0;
    }
    String& operator=(String&& src) noexcept {
        this->~String();
        data_ = src.data_;
        length_ = src.length_;
        capacity_ = src.capacity_;
        src.data_ = null;
        src.length_ = 0;
        src.capacity_ = 0;
        return *this;
    }

    template<Allocator B = A>
    [[nodiscard]] String<B> clone() const noexcept {
        String<B> ret;
        ret.data_ = reinterpret_cast<u8*>(B::alloc(capacity_));
        ret.length_ = length_;
        ret.capacity_ = capacity_;
        Libc::memcpy(ret.data_, data_, length_);
        return ret;
    }

    void set_length(u64 length) noexcept;

    [[nodiscard]] u8& operator[](u64 idx) noexcept;
    [[nodiscard]] const u8& operator[](u64 idx) const noexcept;

    [[nodiscard]] String_View view() const noexcept {
        return String_View{data_, length_};
    }
    [[nodiscard]] String_View sub(u64 start, u64 end) const noexcept;

    [[nodiscard]] u64 write(u64 i, char c) noexcept;
    template<Allocator B>
    [[nodiscard]] u64 write(u64 i, const String<B>& text) noexcept;
    [[nodiscard]] u64 write(u64 i, String_View text) noexcept;

    [[nodiscard]] u8* begin() noexcept {
        return data_;
    }
    [[nodiscard]] u8* end() noexcept {
        return data_ + length_;
    }
    [[nodiscard]] const u8* begin() const noexcept {
        return data_;
    }
    [[nodiscard]] const u8* end() const noexcept {
        return data_ + length_;
    }

    [[nodiscard]] u8* data() noexcept {
        return data_;
    }
    [[nodiscard]] const u8* data() const noexcept {
        return data_;
    }
    [[nodiscard]] u64 length() const noexcept {
        return length_;
    }
    [[nodiscard]] u64 capacity() const noexcept {
        return capacity_;
    }

    [[nodiscard]] bool empty() const noexcept {
        return length_ == 0;
    }

    template<Allocator RA>
    [[nodiscard]] String<RA> terminate() const noexcept;

    template<Allocator RA, Allocator B>
    [[nodiscard]] String<RA> append(const String<B>& next) const noexcept;

private:
    u8* data_ = null;
    u64 length_ = 0;
    u64 capacity_ = 0;

    friend struct String_View;
    friend struct Reflect::Refl<String>;
};

template<Allocator A>
[[nodiscard]] String<A> String_View::string() const noexcept {
    String<A> ret;
    ret.data_ = reinterpret_cast<u8*>(A::alloc(length_));
    ret.length_ = length_;
    ret.capacity_ = length_;
    Libc::memcpy(ret.data_, data_, length_);
    return ret;
}

[[nodiscard]] RPP_FORCE_INLINE String_View operator""_v(const char* c_string,
                                                        size_t length) noexcept {
    return String_View{reinterpret_cast<const u8*>(c_string), length};
}

[[nodiscard]] constexpr bool operator==(String_View l, String_View r) noexcept {
    if(l.length() != r.length()) return false;
    return Libc::strncmp(reinterpret_cast<const char*>(l.data()),
                         reinterpret_cast<const char*>(r.data()), l.length()) == 0;
}

[[nodiscard]] constexpr bool operator<(String_View l, String_View r) noexcept {
    u64 length = l.length() < r.length() ? l.length() : r.length();
    for(u64 i = 0; i < length; i++) {
        if(l[i] < r[i]) return true;
        if(l[i] > r[i]) return false;
    }
    return l.length() < r.length();
}

template<Allocator A>
[[nodiscard]] bool operator==(const String<A>& l, String_View r) noexcept {
    if(l.length() != r.length()) return false;
    return Libc::strncmp(reinterpret_cast<const char*>(l.data()),
                         reinterpret_cast<const char*>(r.data()), l.length()) == 0;
}

template<Allocator B>
[[nodiscard]] bool operator==(String_View l, const String<B>& r) noexcept {
    if(l.length() != r.length()) return false;
    return Libc::strncmp(reinterpret_cast<const char*>(l.data()),
                         reinterpret_cast<const char*>(r.data()), l.length()) == 0;
}

template<Allocator A, Allocator B>
[[nodiscard]] bool operator==(const String<A>& l, const String<B>& r) noexcept {
    if(l.length() != r.length()) return false;
    return Libc::strncmp(reinterpret_cast<const char*>(l.data()),
                         reinterpret_cast<const char*>(r.data()), l.length()) == 0;
}

template<Allocator A, Allocator B>
[[nodiscard]] bool operator<(const String<A>& l, const String<B>& r) noexcept {
    u64 length = l.length() < r.length() ? l.length() : r.length();
    for(u64 i = 0; i < length; i++) {
        if(l[i] < r[i]) return true;
        if(l[i] > r[i]) return false;
    }
    return l.length() < r.length();
}

namespace ascii {

[[nodiscard]] constexpr u8 to_uppercase(u8 c) noexcept {
    if(c >= 'a' && c <= 'z') return c - 'a' + 'A';
    return c;
}
[[nodiscard]] constexpr u8 to_lowercase(u8 c) noexcept {
    if(c >= 'A' && c <= 'Z') return c - 'A' + 'a';
    return c;
}
[[nodiscard]] constexpr bool is_whitespace(u8 c) noexcept {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v';
}

} // namespace ascii

namespace detail {

template<typename T>
struct Is_String {
    constexpr static bool value = false;
};

template<Allocator A>
struct Is_String<String<A>> {
    constexpr static bool value = true;
};

} // namespace detail

template<typename T>
concept Any_String = detail::Is_String<T>::value;

RPP_RECORD(String_View, RPP_FIELD(data_), RPP_FIELD(length_));

template<Allocator A>
RPP_TEMPLATE_RECORD(String, A, RPP_FIELD(data_), RPP_FIELD(length_), RPP_FIELD(capacity_));

namespace Hash {

template<Allocator A>
struct Hash<String<A>> {
    [[nodiscard]] static u64 hash(const String<A>& string) noexcept {
        u64 h = 0;
        for(u8 c : string) h = hash_combine(h, rpp::hash(c));
        return h;
    }
};

template<>
struct Hash<String_View> {
    [[nodiscard]] static u64 hash(const String_View& string) noexcept {
        u64 h = 0;
        for(u8 c : string) h = hash_combine(h, rpp::hash(c));
        return h;
    }
};

} // namespace Hash

} // namespace rpp
