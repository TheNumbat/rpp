
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

template<typename T>
struct Opt {

    Opt() noexcept = default;

    explicit Opt(T&& value) noexcept
        requires Move_Constructable<T>
        : ok_(true), value_(rpp::move(value)) {
    }

    template<typename... Args>
    explicit Opt(Args&&... args) noexcept
        requires Constructable<T, Args...>
        : ok_(true), value_(rpp::forward<Args>(args)...) {
    }

    ~Opt() noexcept {
        if constexpr(Must_Destruct<T>) {
            if(ok_) value_.destruct();
        }
        ok_ = false;
    }

    Opt(const Opt& src) noexcept
        requires Copy_Constructable<T>
    = default;
    Opt& operator=(const Opt& src) noexcept
        requires Copy_Constructable<T>
    = default;

    Opt(Opt&& src) noexcept
        requires Move_Constructable<T>
    {
        ok_ = src.ok_;
        src.ok_ = false;
        if(ok_) {
            value_.construct(rpp::move(*src.value_));
        }
    }

    Opt& operator=(Opt&& src) noexcept
        requires Move_Constructable<T>
    {
        this->~Opt();
        ok_ = src.ok_;
        src.ok_ = false;
        if(ok_) {
            value_.construct(rpp::move(*src.value_));
        }
        return *this;
    }

    Opt& operator=(T&& value) noexcept
        requires Move_Constructable<T>
    {
        this->~Opt();
        value_.construct(rpp::move(value));
        ok_ = true;
        return *this;
    }

    template<typename... Args>
    void emplace(Args&&... args) noexcept
        requires Constructable<T, Args...>
    {
        ok_ = true;
        value_.construct(rpp::forward<Args>(args)...);
    }

    void clear() noexcept {
        if(ok_) {
            value_.destruct();
            ok_ = false;
        }
    }

    Opt clone() const noexcept
        requires Clone<T> || Copy_Constructable<T>
    {
        if(!ok_) return Opt{};
        if constexpr(Clone<T>) {
            return Opt{value_->clone()};
        } else {
            static_assert(Copy_Constructable<T>);
            return Opt{*value_};
        }
    }

    [[nodiscard]] T& value_or(T& other) noexcept {
        if(ok_) return *value_;
        return other;
    }

    [[nodiscard]] T& operator*() noexcept {
        assert(ok_);
        return *value_;
    }
    [[nodiscard]] const T& operator*() const noexcept {
        assert(ok_);
        return *value_;
    }

    [[nodiscard]] T* operator->() noexcept {
        assert(ok_);
        return &*value_;
    }
    [[nodiscard]] const T* operator->() const noexcept {
        assert(ok_);
        return &*value_;
    }

    [[nodiscard]] bool ok() const noexcept {
        return ok_;
    }

protected:
    bool ok_ = false;
    Storage<T> value_;

    friend struct Reflect::Refl<Opt>;
};

template<typename T>
RPP_TEMPLATE_RECORD(Opt, T, RPP_FIELD(ok_), RPP_FIELD(value_));

namespace Format {

template<Reflectable T>
struct Measure<Opt<T>> {
    [[nodiscard]] static u64 measure(const Opt<T>& opt) noexcept {
        if(opt.ok()) return 5 + Measure<T>::measure(*opt);
        return 9;
    }
};
template<Allocator O, Reflectable T>
struct Write<O, Opt<T>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Opt<T>& opt) noexcept {
        if(!opt.ok()) return output.write(idx, "Opt{None}"_v);
        idx = output.write(idx, "Opt{"_v);
        idx = Write<O, T>::write(output, idx, *opt);
        return output.write(idx, '}');
    }
};

} // namespace Format

} // namespace rpp