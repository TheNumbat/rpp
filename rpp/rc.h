
#pragma once

#include "base.h"

namespace rpp {

namespace detail {

template<typename T>
struct Rc_Data {
    u64 references = 0;
    T value;
};

template<typename T>
struct Arc_Data {
    Thread::Atomic references;
    T value;
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
        new(data_) Data{1, T{forward<Args>(args)...}};
    }

    Rc(const Rc& src) = delete;
    Rc& operator=(const Rc& src) = delete;

    Rc dup() const {
        Rc ret;
        ret.data_ = data_;
        if(data_) data_->references++;
        return ret;
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

    friend struct Reflect::Refl<Rc<T>>;
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
        new(data_) Data{Thread::Atomic{1}, T{forward<Args>(args)...}};
    }

    static Arc make()
        requires Default_Constructable<T>
    {
        Arc ret;
        ret.data_ = reinterpret_cast<Data*>(A::alloc(sizeof(Data)));
        new(ret.data_) Data{Thread::Atomic{1}, T{}};
        return ret;
    }

    Arc(const Arc& src) = delete;
    Arc& operator=(const Arc& src) = delete;

    Arc dup() const {
        Arc ret;
        ret.data_ = data_;
        if(data_) data_->references.incr();
        return ret;
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

    friend struct Reflect::Refl<Arc<T>>;
};

namespace Reflect {

template<typename R>
struct Refl<::rpp::detail::Rc_Data<R>> {
    using T = ::rpp::detail::Rc_Data<R>;
    static constexpr Literal name = "Rc_Data";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(value), FIELD(references)>;
};

template<typename R>
struct Refl<Rc<R>> {
    using T = Rc<R>;
    static constexpr Literal name = "Rc";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_)>;
};

template<typename R>
struct Refl<::rpp::detail::Arc_Data<R>> {
    using T = ::rpp::detail::Arc_Data<R>;
    static constexpr Literal name = "Arc_Data";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(value), FIELD(references)>;
};

template<typename R>
struct Refl<Arc<R>> {
    using T = Arc<R>;
    static constexpr Literal name = "Arc";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_)>;
};

} // namespace Reflect

namespace Format {

template<Reflectable T, Allocator A>
struct Measure<Rc<T, A>> {
    static u64 measure(const Rc<T, A>& rc) {
        if(rc)
            return 6 + Measure<T>::measure(*rc) +
                   Measure<decltype(rc.references())>::measure(rc.references());
        return 8;
    }
};
template<Reflectable T, Allocator A>
struct Measure<Arc<T, A>> {
    static u64 measure(const Arc<T, A>& arc) {
        if(arc)
            return 7 + Measure<T>::measure(*arc) +
                   Measure<decltype(arc.references())>::measure(arc.references());
        return 9;
    }
};

template<Allocator O, Reflectable T, Allocator A>
struct Write<O, Rc<T, A>> {
    static u64 write(String<O>& output, u64 idx, const Rc<T, A>& rc) {
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
    static u64 write(String<O>& output, u64 idx, const Arc<T, A>& arc) {
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
