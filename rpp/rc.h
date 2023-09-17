
#pragma once

namespace rpp {

namespace detail {

template<typename T>
struct Rc_Data {
    T value;
    u64 references = 0;
};

template<typename T>
struct Arc_Data {
    T value;
    Thread::Atomic references;
};

} // namespace detail

template<typename T, Allocator A = Mdefault>
struct Rc {
    using Data = detail::Rc_Data<T>;

    Rc() = default;
    ~Rc() {
        drop();
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    explicit Rc(Args&&... args) {
        data_ = reinterpret_cast<Data*>(A::alloc(sizeof(Data)));
        new(data_) Data{T{std::forward<Args>(args)...}, 1};
    }

    Rc(const Rc& src) {
        data_ = src.data_;
        if(data_) data_->references++;
    }
    Rc& operator=(const Rc& src) {
        drop();
        data_ = src.data_;
        if(data_) data_->references++;
    }

    Rc(Rc&& src) {
        data_ = src.data_;
        src.data_ = null;
    }
    Rc& operator=(Rc&& src) {
        drop();
        data_ = src.data_;
        src.data_ = null;
        return *this;
    }

    T* operator->() {
        assert(data_);
        return &data_->value;
    }
    const T* operator->() const {
        assert(data_);
        return &data_->value;
    }
    T& operator*() {
        assert(data_);
        return data_->value;
    }
    const T& operator*() const {
        assert(data_);
        return data_->value;
    }

    operator bool() const {
        return data_ != null;
    }

    u64 references() const {
        return data_ ? data_->references : 0;
    }
    void clear() {
        drop();
    }

private:
    void drop() {
        if(!data_) return;

        data_->references--;
        if(data_->references == 0) {
            data_->~Data();
            A::free(data_);
        }

        data_ = null;
    }

    Data* data_ = null;

    friend struct Reflect<Rc<T>>;
};

template<typename T, Allocator A = Mdefault>
struct Arc {
    using Data = detail::Arc_Data<T>;

    Arc() = default;
    ~Arc() {
        drop();
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    explicit Arc(Args&&... args) {
        data_ = reinterpret_cast<Data*>(A::alloc(sizeof(Data)));
        new(data_) Data{T{std::forward<Args>(args)...}, Thread::Atomic{1}};
    }

    static Arc make()
        requires Default_Constructable<T>
    {
        Arc ret;
        ret.data_ = reinterpret_cast<Data*>(A::alloc(sizeof(Data)));
        new(ret.data_) Data{T{}, Thread::Atomic{1}};
        return ret;
    }

    // NOTE(max): src will not be destroyed while in scope
    Arc(const Arc& src) {
        if(src.data_) {
            src.data_->references.incr();
            data_ = src.data_;
        }
    }
    Arc& operator=(const Arc& src) {
        drop();
        if(src.data_) {
            src.data_->references.incr();
            data_ = src.data_;
        }
    }

    Arc(Arc&& src) {
        data_ = src.data_;
        src.data_ = null;
    }
    Arc& operator=(Arc&& src) {
        drop();
        data_ = src.data_;
        src.data_ = null;
        return *this;
    }

    T* operator->() {
        assert(data_);
        return &data_->value;
    }
    const T* operator->() const {
        assert(data_);
        return &data_->value;
    }
    T& operator*() {
        assert(data_);
        return data_->value;
    }
    const T& operator*() const {
        assert(data_);
        return data_->value;
    }

    operator bool() const {
        return data_ != null;
    }

    u64 references() const {
        return data_ ? data_->references.load() : 0;
    }
    void clear() {
        drop();
    }

private:
    void drop() {
        if(!data_) return;

        if(data_->references.decr() == 0) {
            data_->~Data();
            A::free(data_);
        }

        data_ = null;
    }

    Data* data_ = null;

    friend struct Reflect<Arc<T>>;
};

template<typename R>
struct Reflect<detail::Rc_Data<R>> {
    using T = detail::Rc_Data<R>;
    static constexpr Literal name = "Rc_Data";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(value), FIELD(references)>;
    static_assert(Record<T>);
};

template<typename R>
struct Reflect<Rc<R>> {
    using T = Rc<R>;
    static constexpr Literal name = "Rc";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_)>;
    static_assert(Record<T>);
};

template<typename R>
struct Reflect<detail::Arc_Data<R>> {
    using T = detail::Arc_Data<R>;
    static constexpr Literal name = "Arc_Data";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(value), FIELD(references)>;
    static_assert(Record<T>);
};

template<typename R>
struct Reflect<Arc<R>> {
    using T = Arc<R>;
    static constexpr Literal name = "Arc";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_)>;
    static_assert(Record<T>);
};

} // namespace rpp
