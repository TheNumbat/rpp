
#include "rpp/base.h"

using namespace rpp;

i32 main() {

    // Alloc0
    {
        using A = Mallocator<"Test">;
        void* ptr = A::alloc(100);
        A::free(ptr);
    }

    // Ref0/1
    {
        i32 i = 5;
        Ref<i32> r{i};

        assert(*r == 5);
        i = 10;

        Ref<Ref<i32>> rr{r};
        assert(**rr == 10);
    }

    // String0
    {
        String_View sv = "Hello World"_v;
        String s = sv.string();

        s[0] = 'h';

        for(char c : sv) {
            printf("%c", c);
        }
        printf("\n");
        for(char c : s) {
            printf("%c", c);
        }
        printf("\n");

        String_View sv2 = sv.clone();
        String_View sv3 = std::move(sv2);
        String_View sv4 = s.view();
        String s2 = sv3.string();

        String s3 = s.clone();
        String s4 = std::move(s3);
    }

    // Thread0
    {
        Thread::Mutex mut;
        mut.lock();
        mut.unlock();
        { Thread::Lock lock{mut}; }
    }

    // Log
    {
        info("Info");
        warn("Warn");
    }

    using M = Thread::Mutex;
    using SV = String_View;

    // Pair
    {
        Pair<i32, i32> p{1, 2};
        assert(p.first == 1 && p.second == 2);

        Pair<SV, SV> p1;
        Pair<SV, SV> e{"Hello"_v, "World"_v};
        Pair<SV, SV> e2 = std::move(e);
        Pair<SV, SV> p2 = p1.clone();

        Pair<i32, i32> p3 = std::move(p);
        assert(p3.first == 1 && p3.second == 2);

        Pair<i32, i32> p4 = p3;
        assert(p4.first == 1 && p4.second == 2);

        Pair<M, M> p5;
    }

    // Storage
    {
        Storage<String<>> s;
        s.construct("Hello"_v.string());
        s.destruct();

        Storage<String<>> s2{"World"_v.string()};

        Storage<String<>> s3{s2->clone()};
        Storage<String<>> s4{std::move(*s3)};

        s2.destruct();
        s4.destruct();

        Storage<M> m;
        m.construct();
        m->lock();
        m->unlock();
        m.destruct();
    }

    // Opt
    {
        Opt<i32> o;
        assert(!o);
        o = 5;
        assert(o);

        i32 i = o.get_or(10);
        assert(i == 5);

        Opt<i32> o2 = o;
        Opt<i32> o3 = std::move(o2);

        Opt<SV> os{"Hello"_v};
        Opt<SV> os2 = os.clone();
        Opt<SV> os3 = std::move(os2);

        SV vv = std::move(*os3);
        assert(vv == "Hello"_v);

        const SV& v = os.get_or("World"_v);
        assert(v == "Hello"_v);

        Opt<M> m;
    }

    // Array
    {
        Array<i32, 1> a;
        a[0] = 5;

        Array<i32, 4> b{1, 2, 3, 4};
        Array<i32, 4> c = b;

        assert(c[2] == 3);

        for(i32 i : c) {
            assert(i > 0 && i < 5);
        }

        Array<i32, 4> d = std::move(c);
        assert(d[2] == 3);

        Array<SV, 3> e{"Hello"_v, "World"_v, "!"_v};

        Array<SV, 3> f = e.clone();

        Array<M, 2> m;
    }

    // Vec
    {
        Vec<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);

        assert(v.length() == 3);

        v.pop();
        assert(v.length() == 2);

        Vec<i32> v2 = v.clone();
        Vec<i32> v3 = std::move(v2);

        assert(v3.length() == 2);

        Vec<SV> sv{"Hello"_v, "World"_v};
        Vec<SV> sv2 = sv.clone();
        Vec<SV> sv3 = std::move(sv2);

        assert(sv3.length() == 2);

        Slice<i32> s{v3};
        assert(s.length() == 2);

        Slice<i32> s2 = s.clone();
        Slice<i32> s3 = std::move(s2);

        Slice<SV> s4{sv3};
        assert(s4.length() == 2);
        Slice<SV> s5 = s4.clone();

        Slice<i32> s6 = {1, 2, 3};
        assert(s6.length() == 3);

        Slice<SV> s7 = {"Hello"_v, "World"_v};
        assert(s7.length() == 2);

        Slice<M> smm;
        Slice<M> sm = {M{}};

        Vec<M> m;
        Slice<M> m2{m};
    }

    // Box
    {
        Box<i32> b{5};
        assert(b && *b == 5);

        Box<i32> b2;
        assert(!b2);

        b2 = std::move(b);
        assert(!b && b2 && *b2 == 5);

        Box<i32> b3 = b2.clone();
        assert(b2 && b3 && *b3 == 5);

        Box<SV> sv{"Hello"_v};
        Box<SV> sv2 = sv.clone();
        assert(sv && sv2 && *sv == *sv2 && *sv2 == "Hello"_v);

        Box<M> m;
        m.emplace();
    }

    // Stack
    {
        Stack<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);

        assert(v.length() == 3);

        v.pop();
        assert(v.length() == 2);

        Stack<i32> v2 = v.clone();
        Stack<i32> v3 = std::move(v2);

        assert(v3.length() == 2);

        Stack<SV> sv{"Hello"_v, "World"_v};
        Stack<SV> sv2 = sv.clone();
        Stack<SV> sv3 = std::move(sv2);

        assert(sv3.length() == 2);

        Stack<M> m;
    }

    // Queue
    {
        {
            Queue<String<>> sv{"Hello"_v.string(), "World"_v.string()};
            sv.pop();
        }

        Queue<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);

        assert(v.length() == 3);
        assert(v.front() == 1);

        v.pop();
        assert(v.length() == 2);
        assert(v.front() == 2);

        Queue<i32> v2 = v.clone();
        Queue<i32> v3 = std::move(v2);

        assert(v3.length() == 2);

        Queue<SV> sv{"Hello"_v, "World"_v};
        Queue<SV> sv2 = sv.clone();
        Queue<SV> sv3 = std::move(sv2);

        assert(sv3.length() == 2);

        Queue<M> m;
    }

    // Heap
    {
        {
            Heap<String<>> sv{"Hello"_v.string(), "World"_v.string()};
            sv.pop();
        }

        Heap<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);
        v.push(0);

        assert(v.length() == 4);
        assert(v.top() == 0);

        v.pop();
        assert(v.length() == 3);
        assert(v.top() == 1);

        Heap<i32> v2 = v.clone();
        Heap<i32> v3 = std::move(v2);

        assert(v3.length() == 3);

        Heap<SV> sv{"Hello"_v, "World"_v};
        Heap<SV> sv2 = sv.clone();
        Heap<SV> sv3 = std::move(sv2);

        assert(sv3.length() == 2);

        Heap<SV> o{"a"_v, "aa"_v, "ab"_v, "bb"_v, "aab"_v};
        while(o.length() > 0) {
            o.pop();
        }
    }

    assert(sys_net_allocs() == 0);

    return 0;
}
