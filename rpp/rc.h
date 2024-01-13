
#pragma once

#include "base.h"

namespace rpp {

namespace detail {

template<typename T>
struct Rc_Data {
    explicit Rc_Data(u64 r) noexcept : references(r) {
    }

    u64 references;
    Storage<T> value;
};

template<typename T>
struct Arc_Data {
    explicit Arc_Data(Thread::Atomic r) noexcept
        requires Default_Constructable<T>
        : references(r) {
    }

    Thread::Atomic references;
    Storage<T> value;
};

} // namespace detail

template<typename T, Scalar_Allocator P = Mdefault>
struct Rc {
    using A = Pool_Adaptor<P>;
    using Data = detail::Rc_Data<T>;

    Rc() noexcept = default;
    ~Rc() noexcept {
        drop();
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    explicit Rc(Args&&... args) noexcept {
        data_ = A::template make<Data>(static_cast<u64>(1));
        data_->value.construct(forward<Args>(args)...);
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    [[nodiscard]] static Rc make(Args&&... args) noexcept {
        Rc ret;
        ret.data_ = A::template make<Data>(static_cast<u64>(1));
        ret.data->value.construct(forward<Args>(args)...);
        return ret;
    }

    Rc(const Rc& src) noexcept = delete;
    Rc& operator=(const Rc& src) noexcept = delete;

    [[nodiscard]] Rc dup() const noexcept {
        Rc ret;
        ret.data_ = data_;
        if(data_) data_->references++;
        return ret;
    }

    Rc(Rc&& src) noexcept {
        data_ = src.data_;
        src.data_ = null;
    }
    Rc& operator=(Rc&& src) noexcept {
        drop();
        data_ = src.data_;
        src.data_ = null;
        return *this;
    }

    [[nodiscard]] T* operator->() noexcept {
        assert(data_);
        return &*data_->value;
    }
    [[nodiscard]] const T* operator->() const noexcept {
        assert(data_);
        return &*data_->value;
    }
    [[nodiscard]] T& operator*() noexcept {
        assert(data_);
        return *data_->value;
    }
    [[nodiscard]] const T& operator*() const noexcept {
        assert(data_);
        return *data_->value;
    }

    [[nodiscard]] operator bool() const noexcept {
        return data_ != null;
    }

    [[nodiscard]] u64 references() const noexcept {
        return data_ ? data_->references : 0;
    }
    void clear() noexcept {
        drop();
    }

private:
    void drop() noexcept {
        if(!data_) return;
        data_->references--;
        if(data_->references == 0) {
            data_->value.destruct();
            A::template destroy<Data>(data_);
        }
        data_ = null;
    }

    Data* data_ = null;

    friend struct Reflect::Refl<Rc<T>>;
};

template<typename T, Scalar_Allocator P = Mdefault>
struct Arc {
    using A = Pool_Adaptor<P>;
    using Data = detail::Arc_Data<T>;

    Arc() noexcept = default;
    ~Arc() noexcept {
        drop();
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    explicit Arc(Args&&... args) noexcept {
        data_ = A::template make<Data>(Thread::Atomic{1});
        data_->value.construct(forward<Args>(args)...);
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    [[nodiscard]] static Arc make(Args&&... args) noexcept {
        Arc ret;
        ret.data_ = A::template make<Data>(Thread::Atomic{1});
        ret.data_->value.construct(forward<Args>(args)...);
        return ret;
    }

    Arc(const Arc& src) noexcept = delete;
    Arc& operator=(const Arc& src) noexcept = delete;

    [[nodiscard]] Arc dup() const noexcept {
        Arc ret;
        ret.data_ = data_;
        if(data_) data_->references.incr();
        return ret;
    }

    Arc(Arc&& src) noexcept {
        data_ = src.data_;
        src.data_ = null;
    }
    Arc& operator=(Arc&& src) noexcept {
        drop();
        data_ = src.data_;
        src.data_ = null;
        return *this;
    }

    [[nodiscard]] T* operator->() noexcept {
        assert(data_);
        return &*data_->value;
    }
    [[nodiscard]] const T* operator->() const noexcept {
        assert(data_);
        return &*data_->value;
    }
    [[nodiscard]] T& operator*() noexcept {
        assert(data_);
        return *data_->value;
    }
    [[nodiscard]] const T& operator*() const noexcept {
        assert(data_);
        return *data_->value;
    }

    [[nodiscard]] operator bool() const noexcept {
        return data_ != null;
    }

    [[nodiscard]] u64 references() const noexcept {
        return data_ ? data_->references.load() : 0;
    }
    void clear() {
        drop();
    }

private:
    void drop() noexcept {
        if(!data_) return;
        if(data_->references.decr() == 0) {
            data_->value.destruct();
            A::template destroy<Data>(data_);
        }
        data_ = null;
    }

    Data* data_ = null;

    friend struct Reflect::Refl<Arc<T>>;
};

template<typename T>
RPP_NAMED_TEMPLATE_RECORD(::rpp::detail::Rc_Data, "Rc_Data", T, RPP_FIELD(references),
                          RPP_FIELD(value));

template<typename T>
RPP_NAMED_TEMPLATE_RECORD(::rpp::detail::Arc_Data, "Arc_Data", T, RPP_FIELD(references),
                          RPP_FIELD(value));

template<typename T>
RPP_TEMPLATE_RECORD(Rc, T, RPP_FIELD(data_));

template<typename T>
RPP_TEMPLATE_RECORD(Arc, T, RPP_FIELD(data_));

namespace Format {

template<Reflectable T, Allocator A>
struct Measure<Rc<T, A>> {
    [[nodiscard]] static u64 measure(const Rc<T, A>& rc) noexcept {
        if(rc)
            return 6 + Measure<T>::measure(*rc) +
                   Measure<decltype(rc.references())>::measure(rc.references());
        return 8;
    }
};
template<Reflectable T, Allocator A>
struct Measure<Arc<T, A>> {
    [[nodiscard]] static u64 measure(const Arc<T, A>& arc) noexcept {
        if(arc)
            return 7 + Measure<T>::measure(*arc) +
                   Measure<decltype(arc.references())>::measure(arc.references());
        return 9;
    }
};

template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Rc<T, A>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Rc<T, A>& rc) noexcept {
        if(!rc) return output.write(idx, "Rc{null}"_v);
        idx = output.write(idx, "Rc["_v);
        idx = Write<O, decltype(rc.references())>::write(output, idx, rc.references());
        idx = output.write(idx, "]{"_v);
        idx = Write<O, T>::write(output, idx, *rc);
        return output.write(idx, '}');
    }
};
template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Arc<T, A>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Arc<T, A>& arc) noexcept {
        if(!arc) return output.write(idx, "Arc{null}"_v);
        idx = output.write(idx, "Arc["_v);
        idx = Write<O, decltype(arc.references())>::write(output, idx, arc.references());
        idx = output.write(idx, "]{"_v);
        idx = Write<O, T>::write(output, idx, *arc);
        return output.write(idx, '}');
    }
};

} // namespace Format

} // namespace rpp
