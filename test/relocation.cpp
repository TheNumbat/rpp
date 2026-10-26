
#include "test.h"

#include <rpp/heap.h>

// Every constructed instance needs destruction, including moved-from instances.
struct Live_Value {
    inline static i32 live = 0;
    inline static i32 constructions = 0;

    explicit Live_Value(i32 value) noexcept : value(value) {
        live++;
        constructions++;
    }
    Live_Value(Live_Value&& src) noexcept : Live_Value(src.value) {
    }
    ~Live_Value() noexcept {
        assert(live > 0);
        live--;
    }
    [[nodiscard]] Live_Value clone() const noexcept {
        return Live_Value{value};
    }
    bool operator<(const Live_Value& rhs) const noexcept {
        return value < rhs.value;
    }
    bool operator==(const Live_Value& rhs) const noexcept {
        return value == rhs.value;
    }
    i32 operator()() const noexcept {
        return value;
    }

    i32 value;
};

template<typename Container>
void reserve_lifetimes() {
    {
        Container values;
        values.reserve(8);
        values.emplace(1);
        values.emplace(2);
        assert(Live_Value::live == 2);
        const Live_Value* first = &*values.begin();
        i32 constructions = Live_Value::constructions;
        values.reserve(0);
        values.reserve(1);
        values.reserve(8);
        assert(&*values.begin() == first);
        assert(Live_Value::constructions == constructions);
        values.reserve(32);
        assert(values.length() == 2 && Live_Value::live == 2);
        i32 expected = 1;
        for(const auto& value : values) assert(value.value == expected++);
    }
    assert(Live_Value::live == 0);
}

i32 main() {
    Test test{"empty"_v};
    reserve_lifetimes<Vec<Live_Value>>();
    reserve_lifetimes<Queue<Live_Value>>();
    reserve_lifetimes<Heap<Live_Value>>();
    {
        Queue<Live_Value> values(4);
        for(i32 i = 1; i <= 4; i++) values.emplace(i);
        values.pop();
        values.pop();
        values.emplace(5);
        values.emplace(6);
        assert(Live_Value::live == 4);
        auto first = &values.front();
        values.reserve(2);
        assert(&values.front() == first);
        values.reserve(8);
        assert(Live_Value::live == 4);
        for(u64 i = 0; i < 4; i++) assert(values[i].value == static_cast<i32>(i) + 3);
    }
    assert(Live_Value::live == 0);
    {
        Queue<i32> values(static_cast<u64>(4));
        for(i32 i = 1; i <= 4; i++) values.push(i);
        values.pop();
        values.pop();
        values.push(5);
        values.push(6);
        values.reserve(8);
        for(u64 i = 0; i < 4; i++) assert(values[i] == static_cast<i32>(i) + 3);
    }
    {
        Queue<Live_Value> values(4);
        for(i32 i = 1; i <= 4; i++) values.emplace(i);
        values.pop();
        values.emplace(5); // [2, 3, 4, 5] wraps around the buffer.
        assert(values.erase_first(values[1]));
        assert(values.length() == 3 && Live_Value::live == 3);
        assert(values[0].value == 2 && values[1].value == 4 && values[2].value == 5);
        assert(!values.erase_first(Live_Value{9}));
        assert(values.erase_first(values.front()));
        assert(values.length() == 2 && Live_Value::live == 2);
        assert(values[0].value == 4 && values[1].value == 5);
        assert(values.erase_first(values[1]));
        assert(values.length() == 1 && Live_Value::live == 1);
        assert(values.erase_first(values.front()));
        assert(values.empty() && Live_Value::live == 0);
        assert(!values.erase_first(Live_Value{9}));
        values.emplace(6);
        assert(values.front().value == 6);
    }
    assert(Live_Value::live == 0);
    {
        Queue<String<>> values;
        values.push("one"_v.string());
        values.push("two"_v.string());
        values.push("three"_v.string());
        assert(values.erase_first("one"_v.string()));
        assert(values[0].view() == "two"_v && values[1].view() == "three"_v);
    }
    {
        Queue<i32> values;
        values.push(1);
        values.push(2);
        values.push(3);
        assert(values.erase_first(1));
        assert(values.length() == 2 && values[0] == 2 && values[1] == 3);
    }
    {
        Heap<Live_Value> values;
        for(i32 i = 20; i > 0; i--) {
            values.emplace(i);
            assert(Live_Value::live == 21 - i);
        }
        for(i32 i = 1; i <= 20; i++) {
            assert(values.top().value == i);
            values.pop();
            assert(Live_Value::live == 20 - i);
        }
        assert(values.empty());
    }
    assert(Live_Value::live == 0);
    {
        Map<i32, Live_Value> values;
        for(i32 i = 0; i < 100; i++) {
            values.insert(i, Live_Value{i});
            assert(Live_Value::live == i + 1);
        }
        values.reserve(512);
        assert(Live_Value::live == 100);
        auto first = &values.get(0);
        i32 constructions = Live_Value::constructions;
        values.reserve(0);
        values.reserve(1);
        values.reserve(512);
        assert(&values.get(0) == first);
        assert(Live_Value::constructions == constructions);
        auto copy = values.clone();
        assert(Live_Value::live == 200 && copy.length() == 100);
        for(i32 i = 0; i < 100; i++) assert(copy.get(i).value == i);
        copy.clear();
        assert(Live_Value::live == 100 && copy.empty());
        for(i32 i = 0; i < 100; i++) {
            values.erase(i);
            assert(Live_Value::live == 99 - i);
        }
        assert(values.empty());
        values.insert(1, Live_Value{1});
        values.insert(1, Live_Value{2});
        assert(values.get(1).value == 2 && Live_Value::live == 1);
    }
    assert(Live_Value::live == 0);
    {
        Map<i32, i32> empty;
        auto empty_copy = empty.clone();
        assert(empty_copy.empty());
        Map<i32, i32> values(1);
        values.insert(1, 7);
        values.reserve(5);
        values.insert(2, 9);
        auto copy = values.clone();
        assert(copy.length() == 2 && copy.get(1) == 7 && copy.get(2) == 9);
        copy.clear();
        assert(copy.empty());
        copy.insert(3, 11);
        assert(copy.get(3) == 11);
    }
    {
        Function<i32()> first{Live_Value{1}};
        assert(Live_Value::live == 1);
        Function<i32()> second{move(first)};
        assert(Live_Value::live == 1 && second() == 1);
        Function<i32()> third{Live_Value{3}};
        third = move(second);
        assert(Live_Value::live == 1 && third() == 1);
        auto& self = third;
        third = move(self);
        assert(Live_Value::live == 1 && third() == 1);
        first = Function<i32()>{Live_Value{4}};
        assert(Live_Value::live == 2 && first() == 4);
    }
    assert(Live_Value::live == 0);
    return 0;
}
