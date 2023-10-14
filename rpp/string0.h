
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
        : data_(reinterpret_cast<const u8*>(c_string)), length_(Std::strlen(c_string)) {
    }
    explicit String_View(const Literal& literal)
        : data_(reinterpret_cast<const u8*>(literal.c_string)),
          length_(Std::strlen(literal.c_string)) {
    }
    explicit String_View(const u8* data, u64 length) : data_(data), length_(length) {
    }

    ~String_View() = default;

    String_View(const String_View& src) = default;
    String_View& operator=(const String_View& src) = default;

    String_View(String_View&& src) = default;
    String_View& operator=(String_View&& src) = default;

    const u8& operator[](u64 idx) const;

    String_View file_suffix() const;

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

private:
    const u8* data_;
    u64 length_;

    friend struct Reflect<String_View>;
};

static_assert(Trivial<String_View>);

template<Allocator A>
struct String {

    String() = default;
    explicit String(u64 capacity)
        : data_(reinterpret_cast<u8*>(A::alloc(capacity))), capacity_(capacity), length_(0) {
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
        Std::memcpy(ret.data_, data_, length_);
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

private:
    u8* data_ = null;
    u64 length_ = 0;
    u64 capacity_ = 0;

    friend struct String_View;
    friend struct Reflect<String>;
};

template<Allocator A>
String<A> String_View::string() const {
    String<A> ret;
    ret.data_ = reinterpret_cast<u8*>(A::alloc(length_));
    ret.length_ = length_;
    ret.capacity_ = length_;
    Std::memcpy(ret.data_, data_, length_);
    return ret;
}

inline String_View operator""_v(const char* c_string, size_t length) {
    return String_View{reinterpret_cast<const u8*>(c_string), length};
}

inline bool operator==(String_View l, String_View r) {
    if(l.length() != r.length()) return false;
    return Std::strncmp(reinterpret_cast<const char*>(l.data()),
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

template<Allocator A, Allocator B>
bool operator==(const String<A>& l, const String<B>& r) {
    if(l.length() != r.length()) return false;
    return Std::strncmp(reinterpret_cast<const char*>(l.data()),
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

template<>
struct Reflect<String_View> {
    using T = String_View;
    static constexpr Literal name = "String_View";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_), FIELD(length_)>;
};

template<Allocator A>
struct Reflect<String<A>> {
    using T = String<A>;
    static constexpr Literal name = "String";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_), FIELD(length_), FIELD(capacity_)>;
};

} // namespace rpp
