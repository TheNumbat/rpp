
#pragma once

namespace rpp {

template<Movable T, Allocator A = Mdefault>
struct Stack {

    Stack() = default;
    explicit Stack(u64 capacity) : data_(capacity) {
    }

    template<typename... Ss>
        requires All<T, Ss...> && Move_Constructable<T>
    explicit Stack(Ss&&... init) : data_(std::move(init)...) {
    }

    Stack(const Stack& src) = delete;
    Stack& operator=(const Stack& src) = delete;

    Stack(Stack&& src) = default;
    Stack& operator=(Stack&& src) = default;

    ~Stack() = default;

    template<Allocator B = A>
    Stack<T, B> clone() const
        requires Clone<T> || Trivial<T>
    {
        Stack<T, B> ret;
        ret.data_ = data_.clone();
        return ret;
    }

    u64 length() const {
        return data_.length();
    }
    bool empty() const {
        return data_.empty();
    }
    bool full() const {
        return data_.full();
    }

    T& push(T&& value)
        requires Move_Constructable<T>
    {
        return data_.push(std::move(value));
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    T& emplace(Args&&... args) {
        return data_.emplace(std::forward<Args>(args)...);
    }

    void pop() {
        return data_.pop();
    }

    T& top() {
        return data_.back();
    }
    const T& top() const {
        return data_.back();
    }

    void clear() {
        data_.clear();
    }

    const T* begin() const {
        return data_.begin();
    }
    const T* end() const {
        return data_.end();
    }
    T* begin() {
        return data_.begin();
    }
    T* end() {
        return data_.end();
    }

private:
    Vec<T, A> data_;

    friend struct Reflect<Stack>;
};

template<typename S, Allocator A>
struct Reflect<Stack<S, A>> {
    using T = Stack<S, A>;
    static constexpr Literal name = "Stack";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_)>;
    static_assert(Record<T>);
};

} // namespace rpp
