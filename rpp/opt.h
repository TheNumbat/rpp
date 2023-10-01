
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

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
        requires Clone<T> || Trivial<T>
    {
        if(!ok_) return Opt{};
        if constexpr(Clone<T>)
            return Opt{value_.clone()};
        else
            return Opt{*value_};
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
};

namespace Format {

template<Reflectable T>
struct Measure<Opt<T>> {
    static u64 measure(const Opt<T>& opt) {
        if(opt) return 5 + Measure<T>::measure(*opt);
        return 9;
    }
};
template<Allocator O, Reflectable T>
struct Write<O, Opt<T>> {
    static u64 write(String<O>& output, u64 idx, const Opt<T>& opt) {
        if(!opt) return output.write(idx, "Opt{None}"_v);
        idx = output.write(idx, "Opt{"_v);
        idx = Write<O, T>::write(output, idx, *opt);
        return output.write(idx, '}');
    }
};

} // namespace Format

} // namespace rpp