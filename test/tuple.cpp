
#include "test.h"

#include <tuple.h>

i32 main() {
    Test test{"empty"_v};
    Trace("Pair") {
        Pair<i32, i32> p{1, 2};
        assert(p.first == 1 && p.second == 2);

        Pair<String_View, String_View> p1;
        Pair<String_View, String_View> e{"Hello"_v, "World"_v};
        Pair<String_View, String_View> e2 = move(e);
        Pair<String_View, String_View> p2 = p1.clone();
        (void)e2;
        (void)p2;

        Pair<i32, i32> p3 = move(p);
        assert(p3.first == 1 && p3.second == 2);

        Pair<i32, i32> p4 = p3;
        assert(p4.first == 1 && p4.second == 2);

        Pair<Thread::Mutex, Thread::Mutex> p5;

        auto [a, b] = p4;
        assert(a == 1 && b == 2);

        auto& [c, d] = p4;
        assert(c == 1 && d == 2);

        auto&& [e_, f] = p4;
        assert(e_ == 1 && f == 2);
    }
    Trace("Tuple") {
        Tuple<> t0;
        Tuple<i32> t1{1};
        Tuple<i32, i32> t2{1, 2};
        Tuple<i32, i32, i32> t3{1, 2, 3};

        assert(t0.length() == 0);
        assert(t1.length() == 1);
        assert(t2.length() == 2);
        assert(t3.length() == 3);

        assert(t1.get<0>() == 1);
        assert(t2.get<0>() == 1);
        assert(t2.get<1>() == 2);
        assert(t3.get<0>() == 1);
        assert(t3.get<1>() == 2);
        assert(t3.get<2>() == 3);

        Tuple<i32, i32> t4 = t2.clone();
        assert(t4.get<0>() == 1);

        Tuple<i32, i32> t5 = move(t4);
        assert(t5.get<0>() == 1);

        Tuple<i32, i32> t6 = t5;
        assert(t6.get<0>() == 1);

        auto [a, b] = t6;
        assert(a == 1 && b == 2);

        auto& [c, d] = t6;
        assert(c == 1 && d == 2);

        auto&& [e, f] = t6;
        assert(e == 1 && f == 2);

        Tuple<i32, String_View, String<>> t7{1, "Hello"_v, "World"_v.string()};

        Tuple<i32, String_View, String<>> t8 = t7.clone();
        Tuple<i32, String_View, String<>> t9 = move(t8);

        assert(t9.get<0>() == 1);
        assert(t9.get<1>() == "Hello"_v);
        assert(t9.get<2>() == "World"_v.string());

        auto [g, h, i] = move(t9);
        assert(g == 1 && h == "Hello"_v && i == "World"_v.string());

        Tuple<Thread::Mutex, Thread::Mutex> t10;
    }
    return 0;
}
