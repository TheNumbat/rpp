
#pragma once

namespace rpp {

template<typename T>
struct Opt {

    Opt() = default;

    explicit Opt(T&& value)
        requires Move_Constructable<T>
        : value_(std::move(value)), ok_(true) {
    }

    template<typename... Args>
    explicit Opt(Args&&... args)
        requires Constructable<T, Args...>
        : value_(std::forward<Args>(args)...), ok_(true) {
    }

    ~Opt() {
        if constexpr(Must_Destruct<T>) {
            if(ok_) value_.destruct();
        }
        ok_ = false;
    }

    Opt(const Opt& src)
        requires Trivial<T>
    = default;
    Opt& operator=(const Opt& src)
        requires Trivial<T>
    = default;

    Opt(Opt&& src)
        requires Move_Constructable<T>
    {
        ok_ = src.ok_;
        src.ok_ = false;
        if(ok_) {
            value_.construct(std::move(*src.value_));
        }
    }

    Opt& operator=(Opt&& src)
        requires Move_Constructable<T>
    {
        this->~Opt();
        ok_ = src.ok_;
        src.ok_ = false;
        if(ok_) {
            value_.construct(std::move(*src.value_));
        }
        return *this;
    }

    Opt& operator=(T&& value)
        requires Move_Constructable<T>
    {
        this->~Opt();
        value_.construct(std::move(value));
        ok_ = true;
        return *this;
    }

    Opt clone() const
        requires Clone<T>
    {
        if(!ok_) return Opt{};
        return Opt{value_->clone()};
    }

    T& operator*() {
        assert(ok_);
        return *value_;
    }
    const T& operator*() const {
        assert(ok_);
        return *value_;
    }

    T* operator->() {
        assert(ok_);
        return &*value_;
    }
    const T* operator->() const {
        assert(ok_);
        return &*value_;
    }

    operator bool() const {
        return ok_;
    }

protected:
    bool ok_ = false;
    Storage<T> value_;

    friend struct Reflect<Opt>;
};

template<typename O>
struct Reflect<Opt<O>> {
    using T = Opt<O>;
    static constexpr Literal name = "Opt";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(ok_), FIELD(value_)>;
    static_assert(Record<T>);
};

} // namespace rpp