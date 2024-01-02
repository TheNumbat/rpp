
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<Allocator A = Mdefault>
struct String;

struct String_View {

    String_View() = default;

    explicit String_View(const char* c_string)
        : data_(reinterpret_cast<const u8*>(c_string)), length_(Libc::strlen(c_string)) {
    }

    explicit String_View(const Literal& literal)
        : data_(reinterpret_cast<const u8*>(literal.c_string)) {
        while(length_ < Literal::max_len && literal.c_string[length_]) length_++;
    }

    explicit String_View(const u8* data, u64 length) : data_(data), length_(length) {
    }

    ~String_View() = default;

    String_View(const String_View& src) = default;
    String_View& operator=(const String_View& src) = default;

    String_View(String_View&& src) = default;
    String_View& operator=(String_View&& src) = default;

    const u8& operator[](u64 idx) const;

    constexpr String_View file_suffix() const;
    constexpr String_View file_extension() const;
    constexpr String_View remove_file_suffix() const;

    template<Allocator A = Mdefault>
    String<A> string() const;

    const u8* begin() const {
        return data_;
    }
    const u8* end() const {
        return data_ + length_;
    }

    const u8* data() const {
        return data_;
    }
    u64 length() const {
        return length_;
    }

    bool empty() const {
        return length_ == 0;
    }

    template<Allocator A>
    String<A> terminate() const;

    template<Allocator A>
    String<A> append(String_View next) const;

    String_View sub(u64 start, u64 end) const;

    String_View clone() const {
        return String_View{data_, length_};
    }

private:
    const u8* data_ = null;
    u64 length_ = 0;

    friend struct Reflect::Refl<String_View>;
};

template<Allocator A>
struct String {

    String() = default;
    explicit String(u64 capacity)
        : data_(reinterpret_cast<u8*>(A::alloc(capacity))), length_(0), capacity_(capacity) {
    }

    ~String() {
        A::free(data_);
        data_ = null;
        capacity_ = 0;
        length_ = 0;
    }

    String(const String& src) = delete;
    String& operator=(const String& src) = delete;

    String(String&& src) : data_(src.data_), length_(src.length_), capacity_(src.capacity_) {
        src.data_ = null;
        src.length_ = 0;
        src.capacity_ = 0;
    }
    String& operator=(String&& src) {
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
    String<B> clone() const {
        String<B> ret;
        ret.data_ = reinterpret_cast<u8*>(B::alloc(capacity_));
        ret.length_ = length_;
        ret.capacity_ = capacity_;
        Libc::memcpy(ret.data_, data_, length_);
        return ret;
    }

    void set_length(u64 length);

    u8& operator[](u64 idx);
    const u8& operator[](u64 idx) const;

    String_View view() const {
        return String_View{data_, length_};
    }
    String_View sub(u64 start, u64 end) const;

    u64 write(u64 i, char c);
    template<Allocator B>
    u64 write(u64 i, const String<B>& text);
    u64 write(u64 i, String_View text);

    u8* begin() {
        return data_;
    }
    u8* end() {
        return data_ + length_;
    }
    const u8* begin() const {
        return data_;
    }
    const u8* end() const {
        return data_ + length_;
    }

    u8* data() {
        return data_;
    }
    const u8* data() const {
        return data_;
    }
    u64 length() const {
        return length_;
    }
    u64 capacity() const {
        return capacity_;
    }

    bool empty() const {
        return length_ == 0;
    }

    template<Allocator RA>
    String<RA> terminate() const;

    template<Allocator RA, Allocator B>
    String<RA> append(const String<B>& next) const;

private:
    u8* data_ = null;
    u64 length_ = 0;
    u64 capacity_ = 0;

    friend struct String_View;
    friend struct Reflect::Refl<String>;
};

template<Allocator A>
String<A> String_View::string() const {
    String<A> ret;
    ret.data_ = reinterpret_cast<u8*>(A::alloc(length_));
    ret.length_ = length_;
    ret.capacity_ = length_;
    Libc::memcpy(ret.data_, data_, length_);
    return ret;
}

inline String_View operator""_v(const char* c_string, size_t length) {
    return String_View{reinterpret_cast<const u8*>(c_string), length};
}

inline bool operator==(String_View l, String_View r) {
    if(l.length() != r.length()) return false;
    return Libc::strncmp(reinterpret_cast<const char*>(l.data()),
                         reinterpret_cast<const char*>(r.data()), l.length()) == 0;
}

inline bool operator<(String_View l, String_View r) {
    u64 length = l.length() < r.length() ? l.length() : r.length();
    for(u64 i = 0; i < length; i++) {
        if(l[i] < r[i]) return true;
        if(l[i] > r[i]) return false;
    }
    return l.length() < r.length();
}

template<Allocator A>
bool operator==(const String<A>& l, String_View r) {
    if(l.length() != r.length()) return false;
    return Libc::strncmp(reinterpret_cast<const char*>(l.data()),
                         reinterpret_cast<const char*>(r.data()), l.length()) == 0;
}

template<Allocator B>
bool operator==(String_View l, const String<B>& r) {
    if(l.length() != r.length()) return false;
    return Libc::strncmp(reinterpret_cast<const char*>(l.data()),
                         reinterpret_cast<const char*>(r.data()), l.length()) == 0;
}

template<Allocator A, Allocator B>
inline bool operator==(const String<A>& l, const String<B>& r) {
    if(l.length() != r.length()) return false;
    return Libc::strncmp(reinterpret_cast<const char*>(l.data()),
                         reinterpret_cast<const char*>(r.data()), l.length()) == 0;
}

template<Allocator A, Allocator B>
bool operator<(const String<A>& l, const String<B>& r) {
    u64 length = l.length() < r.length() ? l.length() : r.length();
    for(u64 i = 0; i < length; i++) {
        if(l[i] < r[i]) return true;
        if(l[i] > r[i]) return false;
    }
    return l.length() < r.length();
}

namespace ascii {

constexpr u8 to_uppercase(u8 c) {
    if(c >= 'a' && c <= 'z') return c - 'a' + 'A';
    return c;
}
constexpr u8 to_lowercase(u8 c) {
    if(c >= 'A' && c <= 'Z') return c - 'A' + 'a';
    return c;
}
constexpr bool is_whitespace(u8 c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v';
}

} // namespace ascii

namespace detail {

template<typename T>
struct Is_String {
    static constexpr bool value = false;
};

template<Allocator A>
struct Is_String<String<A>> {
    static constexpr bool value = true;
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
    static u64 hash(const String<A>& string) {
        u64 h = 0;
        for(u8 c : string) h = hash_combine(h, rpp::hash(c));
        return h;
    }
};

template<>
struct Hash<String_View> {
    static u64 hash(const String_View& string) {
        u64 h = 0;
        for(u8 c : string) h = hash_combine(h, rpp::hash(c));
        return h;
    }
};

} // namespace Hash

} // namespace rpp
