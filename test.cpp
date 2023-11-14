
#include "rpp/base.h"

#include "rpp/files.h"
#include "rpp/net.h"

#include "rpp/async.h"
#include "rpp/asyncio.h"
#include "rpp/pool.h"
#include "rpp/thread.h"

#include "rpp/range_allocator.h"
#include "rpp/vmath.h"

#include "rpp/function.h"
#include "rpp/heap.h"
#include "rpp/rc.h"
#include "rpp/stack.h"
#include "rpp/tuple.h"
#include "rpp/variant.h"

using namespace rpp;

struct Ints {
    i32 i;
    u16 u;
};
struct Vecs {
    Vec<i32> i;
    Vec<u32> u;
};
namespace rpp {
template<>
struct rpp::detail::Reflect<Ints> {
    using T = Ints;
    static constexpr Literal name = "Ints";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(i), FIELD(u)>;
};
template<>
struct rpp::detail::Reflect<Vecs> {
    using T = Vecs;
    static constexpr Literal name = "Vecs";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(i), FIELD(u)>;
};
} // namespace rpp

auto lots_of_jobs(Thread::Pool<>& pool, u64 depth) -> Async::Task<u64> {
    if(depth == 0) {
        co_return 1;
    }
    co_await pool.suspend();
    auto job0 = lots_of_jobs(pool, depth - 1);
    auto job1 = lots_of_jobs(pool, depth - 1);
    co_return co_await job0 + co_await job1;
};

i32 main() {

    Thread::set_priority(Thread::Priority::high);
    Mregion::create();
    Profile::start_thread();
    Profile::begin_frame();

    [] {
        Region_Scope;
        Range_Allocator<Mregion>::test();
    }();

    // Alloc0
    [] {
        Prof_Scope("Alloc0");

        using A = Mallocator<"Test">;
        void* ptr = A::alloc(100);
        A::free(ptr);
    }();

    // Ref0/1
    [] {
        Prof_Scope("Ref0/1");

        i32 i = 5;
        Ref<i32> r{i};

        assert(*r == 5);
        i = 10;

        Ref<Ref<i32>> rr{r};
        assert(**rr == 10);
    }();

    // String0
    [] {
        Prof_Scope("String0");

        String_View sv = "Hello World"_v;
        String s = sv.string();

        s[0] = 'h';

        u64 i = 0;
        for(char c : sv) {
            assert(sv[i++] == c);
        }
        i = 0;
        for(char c : s) {
            assert(s[i++] == c);
        }

        String_View sv2 = sv;
        String_View sv3 = std::move(sv2);
        String_View sv4 = s.view();
        String s2 = sv3.string();

        String s3 = s.clone();
        String s4 = std::move(s3);

        (void)s4;
        (void)sv4;
    }();

    // Thread0
    [] {
        Prof_Scope("Thread0");

        Thread::Mutex mut;
        mut.lock();
        mut.unlock();
        { Thread::Lock lock{mut}; }
    }();

    // Log
    [] {
        Prof_Scope("Log");

        info("Info");
        warn("Warn");
    }();

    using M = Thread::Mutex;
    using SV = String_View;

    // Pair
    [] {
        Prof_Scope("Pair");

        Pair<i32, i32> p{1, 2};
        assert(p.first == 1 && p.second == 2);

        Pair<SV, SV> p1;
        Pair<SV, SV> e{"Hello"_v, "World"_v};
        Pair<SV, SV> e2 = std::move(e);
        Pair<SV, SV> p2 = p1.clone();
        (void)e2;
        (void)p2;

        Pair<i32, i32> p3 = std::move(p);
        assert(p3.first == 1 && p3.second == 2);

        Pair<i32, i32> p4 = p3;
        assert(p4.first == 1 && p4.second == 2);

        Pair<M, M> p5;

        auto [a, b] = p4;
        assert(a == 1 && b == 2);

        auto& [c, d] = p4;
        assert(c == 1 && d == 2);

        auto&& [e_, f] = p4;
        assert(e_ == 1 && f == 2);
    }();

    // Tuple
    [] {
        Prof_Scope("Tuple");

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

        Tuple<i32, i32> t5 = std::move(t4);
        assert(t5.get<0>() == 1);

        Tuple<i32, i32> t6 = t5;
        assert(t6.get<0>() == 1);

        auto [a, b] = t6;
        assert(a == 1 && b == 2);

        auto& [c, d] = t6;
        assert(c == 1 && d == 2);

        auto&& [e, f] = t6;
        assert(e == 1 && f == 2);

        Tuple<i32, SV, String<>> t7{1, "Hello"_v, "World"_v.string()};

        Tuple<i32, SV, String<>> t8 = t7.clone();
        Tuple<i32, SV, String<>> t9 = std::move(t8);

        assert(t9.get<0>() == 1);
        assert(t9.get<1>() == "Hello"_v);
        assert(t9.get<2>() == "World"_v.string());

        auto [g, h, i] = std::move(t9);
        info("%", t9.get<2>());
        assert(g == 1 && h == "Hello"_v && i == "World"_v.string());

        Tuple<M, M> t10;
    }();

    // Variant
    [] {
        static_assert(alignof(i32) == 4);
        Variant<i32> v{1};
        info("sizeof variant %", sizeof(v));

        v.match([](i32 i) { info("variant has %", i); });
        v.match([](i32& i) { info("variant has %", i); });

        Variant<i32, SV> v2{"Hello"_v};

        v2.match(Overload{[](i32 i) { info("variant has int %", i); },
                          [](SV s) { info("variant has string view %", s); }});

        String<> i = v2.match(Overload{[](i32 i) {
                                           info("variant has int %", i);
                                           return "int"_v.string();
                                       },
                                       [](SV s) {
                                           info("variant has string view %", s);
                                           return "string"_v.string();
                                       }});

        info("returned % from match", i);

        Variant<i32, SV> v3 = v2.clone();
        assert(v3.index() == 1);
        Variant<i32, SV> v4 = std::move(v3);
        assert(v3.index() == 1);

        { //
            Variant<i32, String<>> v5{"Hello"_v.string()};
            Variant<i32, String<>> v6 = v5.clone();
            Variant<i32, String<>> v7 = std::move(v6);

            Variant<i32, String<>> v8{1};
            Variant<i32, String<>> v9 = v8.clone();
            Variant<i32, String<>> v10 = std::move(v9);
        }

        Variant<Named<"1", i32>, Named<"2", i32>> v11{Named<"1", i32>{1}};

        v11.match([](const auto& i) {
            info("variant has %: %", String_View{Decay<decltype(i)>::type::name}, i);
        });
    }();

    // Storage
    [] {
        Prof_Scope("Storage");

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
    }();

    // Opt
    [] {
        Prof_Scope("Opt");

        Opt<i32> o;
        assert(!o);
        o = 5;
        assert(o);

        Opt<i32> o2 = o;
        Opt<i32> o3 = std::move(o2);

        Opt<SV> os{"Hello"_v};
        Opt<SV> os2 = os.clone();
        Opt<SV> os3 = std::move(os2);

        SV vv = std::move(*os3);
        assert(vv == "Hello"_v);

        assert(os->length() == 5);

        Opt<M> m;
    }();

    // Array
    [] {
        Prof_Scope("Array");

        Array<i32, 1> a;
        a[0] = 5;

        Array<i32, 4> b{1, 2, 3, 4};
        Array<i32, 4> c = b;

        assert(c[2] == 3);

        u64 id = 0;
        for(i32 i : c) {
            assert(i == c[id++]);
        }
        id = 0;
        for(i32& i : c) {
            assert(i == c[id++]);
        }

        const Array<i32, 4>& cc = c;

        id = 0;
        for(i32 i : cc) {
            assert(i == cc[id++]);
        }
        id = 0;
        for(const i32& i : cc) {
            assert(i == cc[id++]);
        }

        Array<i32, 4> d = std::move(c);
        assert(d[2] == 3);

        Array<SV, 3> e{"Hello"_v, "World"_v, "!"_v};

        Array<SV, 3> f = e.clone();

        Array<M, 2> m;

        (void)f;
    }();

    // Vec
    [] {
        Prof_Scope("Vec");

        Vec<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);

        assert(v.length() == 3);

        u64 id = 0;
        for(i32 i : v) {
            assert(i == v[id++]);
        }
        id = 0;
        for(i32& i : v) {
            assert(i == v[id++]);
        }

        const auto& constv = v;
        id = 0;
        for(i32 i : constv) {
            assert(i == constv[id++]);
        }
        id = 0;
        for(const i32& i : constv) {
            assert(i == constv[id++]);
        }

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

        Slice<i32> s2 = s;
        Slice<i32> s3 = std::move(s2);

        Slice<SV> s4{sv3};
        assert(s4.length() == 2);
        Slice<SV> s5 = s4;

        Slice<i32> s6{1, 2, 3};
        assert(s6.length() == 3);

        Slice<SV> s7{"Hello"_v, "World"_v};
        assert(s7.length() == 2);

        Slice<M> smm;
        (void)smm;
        Slice<M> sm{M{}};

        Slice<M> m2{};

        Vec<Function<void()>> vf;
        for(i32 i = 0; i < 10; i++) {
            vf.push([]() { info("Hello"); });
        }

        (void)m2;
        (void)s3;
        (void)s5;
    }();

    // Box
    [] {
        Prof_Scope("Box");

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
    }();

    // Stack
    [] {
        Prof_Scope("Stack");

        Stack<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);

        assert(v.length() == 3);

        i32 sum = 0;
        for(i32 i : v) {
            sum += i;
        }
        assert(sum == 6);

        sum = 0;
        for(i32& i : v) {
            sum += i;
        }
        assert(sum == 6);

        const auto& constv = v;

        sum = 0;
        for(i32 i : constv) {
            sum += i;
        }
        assert(sum == 6);
        sum = 0;
        for(const i32& i : constv) {
            sum += i;
        }
        assert(sum == 6);

        v.pop();
        assert(v.length() == 2);

        Stack<i32> v2 = v.clone();
        Stack<i32> v3 = std::move(v2);

        assert(v3.length() == 2);

        Stack<SV> sv{"Hello"_v, "World"_v};
        Stack<SV> sv2 = sv.clone();
        Stack<SV> sv3 = std::move(sv2);

        assert(sv3.length() == 2);

        Stack<Function<void()>> vf;
        for(i32 i = 0; i < 10; i++) {
            vf.push([]() { info("Hello"); });
        }
    }();

    // Queue
    [] {
        Prof_Scope("Queue");

        {
            Queue<String<>> sv{"Hello"_v.string(), "World"_v.string()};
            sv.pop();
        }

        Queue<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);

        i32 sum = 0;
        for(i32 i : v) {
            sum += i;
        }
        assert(sum == 6);
        sum = 0;
        for(i32& i : v) {
            sum += i;
        }
        assert(sum == 6);

        const auto& constv = v;
        sum = 0;
        for(i32 i : constv) {
            sum += i;
        }
        assert(sum == 6);
        sum = 0;
        for(const i32& i : constv) {
            sum += i;
        }
        assert(sum == 6);

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

        Queue<Function<void()>> vf;
        for(i32 i = 0; i < 10; i++) {
            vf.push([]() { info("Hello"); });
        }
    }();

    // Heap
    [] {
        Prof_Scope("Heap");

        {
            Heap<String<>> sv{"Hello"_v.string(), "World"_v.string()};
            sv.pop();
        }

        Heap<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);
        v.push(0);

        i32 sum = 0;
        for(i32 i : v) {
            sum += i;
        }
        assert(sum == 6);
        sum = 0;
        for(i32& i : v) {
            sum += i;
        }
        assert(sum == 6);

        const auto& constv = v;
        sum = 0;
        for(i32 i : constv) {
            sum += i;
        }
        assert(sum == 6);
        sum = 0;
        for(const i32& i : constv) {
            sum += i;
        }
        assert(sum == 6);

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

        struct F {
            Function<void()> f;
            bool operator<(const F& other) const {
                return true;
            }
        };

        Heap<F> vf;
        for(i32 i = 0; i < 10; i++) {
            vf.push({[]() { info("Hello"); }});
        }
    }();

    // Map
    [] {
        Prof_Scope("Map");

        {
            Map<String<>, String<>> sv{Pair{"Hello"_v.string(), "World"_v.string()}};
            //
        }

        Map<i32, i32> v;
        v.insert(1, 1);
        v.insert(2, 2);
        v.insert(3, 3);

        for(auto [k, vv] : v) {
            vv = k;
            info("% %", k, vv);
        }
        for(auto& [k, vv] : v) {
            vv = k;
            info("% %", k, vv);
        }

        const auto& constv = v;
        for(auto [k, vv] : constv) {
            k = 0;
            vv = k;
            info("% %", k, vv);
        }
        for(const auto& [k, vv] : constv) {
            info("% %", k, vv);
        }

        assert(v.length() == 3);
        assert(v.get(1) == 1);

        v.erase(2);
        assert(v.length() == 2);
        assert(v.get(3) == 3);

        Map<i32, i32> v2 = v.clone();
        Map<i32, i32> v3 = std::move(v2);

        assert(v3.length() == 2);

        Map<i32, String_View> i_sv{Pair{1, "Hello"_v}, Pair{2, "World"_v}};
        Map<SV, i32> sv_i{Pair{"Hello"_v, 1}, Pair{"World"_v, 2}};

        Map<SV, SV> sv{Pair{"Hello"_v, "World"_v}};

        Map<SV, SV> sv2 = sv.clone();
        Map<SV, SV> sv3 = std::move(sv2);

        assert(sv3.length() == 1);

        Map<i32, Function<void()>> ff;
        for(i32 i = 0; i < 40; i++) {
            ff.insert(i, []() { info("Hello"); });
        }
    }();

    // Rc
    [] {
        Rc<i32> r0;
        assert(!r0);

        Rc<i32> r1{5};
        assert(r1 && *r1 == 5);
        assert(r1.references() == 1);

        Rc<i32> r2 = r1.dup();
        assert(r1 && r2 && *r1 == 5 && *r2 == 5);
        assert(r1.references() == 2 && r2.references() == 2);

        Rc<i32> r3 = std::move(r2);
        assert(r1 && r3 && *r1 == 5 && *r3 == 5);
        assert(r1.references() == 2 && r3.references() == 2);
        assert(!r2 && r2.references() == 0);
    }();

    // Arc
    [] {
        Arc<i32> r0;
        assert(!r0);

        Arc<i32> r1{5};
        assert(r1 && *r1 == 5);
        assert(r1.references() == 1);

        Arc<i32> r2 = r1.dup();
        assert(r1 && r2 && *r1 == 5 && *r2 == 5);
        assert(r1.references() == 2 && r2.references() == 2);

        Arc<i32> r3 = std::move(r2);
        assert(r1 && r3 && *r1 == 5 && *r3 == 5);
        assert(r1.references() == 2 && r3.references() == 2);
        assert(!r2 && r2.references() == 0);
    }();

    // Function
    [] {
        Function<void()> f{
            []() { info("Hello function"); },
        };

        f();

        Function<void()> f2 = std::move(f);

        f2();

        String<> test = "Hello"_v.string();

        Function<void()> f3{
            [&test]() { info("Hello function 2: %", test); },
        };

        f3();

        Function<void()> f4{
            [test = std::move(test)]() { info("Hello function 2: %", test); },
        };

        f4();

        Function<void()> f5 = std::move(f4);

        f5();

        Function<i32(i32, i32)> f6{
            [](i32 a, i32 b) { return a + b; },
        };

        info("f6(1, 2): %", f6(1, 2));
        assert(f6(1, 2) == 3);

        Function<i32(i32, i32&, const i32&, i32&&)> f7{
            [](i32 a, i32& b, const i32& c, i32&& d) { return a + b + c + d; },
        };

        i32 i = 2;
        assert(f7(1, i, 3, 4) == 10);

        Function<void(String_View, String_View&, const String_View&, String_View&&)> f8{
            [](String_View a, String_View& b, const String_View& c, String_View&& d) {
                info("% % % %", a, b, c, d);
            },
        };

        String_View s = "Hello"_v;
        String_View s2 = "World"_v;
        f8(s, s, s, std::move(s2));

        struct Large {
            char data[32];
        };
        Large large;

        struct Larger {
            char data[64];
        };
        Larger larger;

        Function<void()> f9{
            [large]() {
                (void)large;
                info("Hello function 3");
            },
        };

        FunctionN<8, void()> f10{
            [larger]() {
                (void)larger;
                info("Hello function 3");
            },
        };
    }();

    // Format
    [] {
        info("%%");

        info("%", true);
        info("%", false);

        i32 array[2] = {1, 2};
        info("% %", format_typename<decltype(array)>(), array);

        info("% %", format_typename<Ints>(), Ints{10, 2});
        // Ints{i : 10, u : 2}

        info("% %", format_typename<Kind>(), Kind::enum_);

        info("%", "Hello"_v);
        info("%", "Hello"_v.string());

        info("% %", format_typename<Ref<i32>>(), Ref<i32>{});

        i32 i = 0;
        info("%", Ref{i});

        info("%", null);
        info("%", &i);

        info("% %", format_typename<Box<i32>>(), Box<i32>{});
        info("%", Box<i32>{i});

        info("% %", format_typename<Pair<i32, f32>>(), Pair{23, 13.0f});

        info("% %", format_typename<Tuple<>>(), Tuple<>{});
        info("% %", format_typename<Tuple<f32, i32, String_View>>(),
             Tuple<f32, i32, String_View>{1.0f, 2, "Hello"_v});

        info("% %", format_typename<Variant<i32, f32>>(), Variant<i32, f32>{1});
        info("%", Variant<i32, f32>{1.0f});
        info("% %", format_typename<Variant<Named<"One", i32>, Named<"Two", i32>>>(),
             Variant<Named<"One", i32>, Named<"Two", i32>>{Named<"One", i32>{1}});
        info("%", Variant<Named<"One", i32>, Named<"Two", i32>>{Named<"Two", i32>{2}});

        info("% %", format_typename<Function<void()>>(), Function<void()>{});
        info("% %", format_typename<Function<void()>>(), Function<void()>{[]() {}});
        info("% %", format_typename<Function<void(i32)>>(), Function<void(i32)>{[](i32 i) {}});
        info("% %", format_typename<Function<void(i32, i32)>>(),
             Function<void(i32, i32)>{[](i32 i, i32 j) {}});
        info("% %", format_typename<Function<void(i32, i32)>>(),
             Function<i32(i32, i32)>{[](i32 i, i32 j) { return i + j; }});
        // Only prints "Function" as the argument type, seems fine for now
        info("% %", format_typename<Function<i32(Function<Function<void()>(f32)>)>>(),
             Function<i32(Function<Function<void()>(f32)>)>{[](Function<Function<void()>(f32)> f) {
                 f(1.0f)();
                 return 0;
             }});

        info("% %", format_typename<Storage<Ints>>(), Storage<Ints>{});

        info("% %", format_typename<Opt<i32>>(), Opt<i32>{});
        info("%", Opt<i32>{5});

        info("% %", format_typename<Array<i32, 2>>(), Array<i32, 2>{1, 2});
        info("%", Array<i32, 2>{});

        info("% %", format_typename<Vec<i32>>(), Vec<i32>{1, 2});
        info("%", Vec<i32>{});

        info("% %", format_typename<Stack<i32>>(), Stack<i32>{1, 2});
        info("%", Stack<i32>{});

        info("% %", format_typename<Queue<i32>>(), Queue<i32>{1, 2});
        info("%", Queue<i32>{});

        info("% %", format_typename<Heap<i32>>(), Heap<i32>{1, 2});
        info("%", Heap<i32>{});

        info("% %", format_typename<Map<i32, i32>>(), Map<i32, i32>{Pair{1, 2}, Pair{3, 4}});
        info("%", Map<i32, i32>{});

        info("% %", format_typename<Rc<i32>>(), Rc<i32>{});
        info("%", Rc<i32>{5});

        info("% %", format_typename<Arc<i32>>(), Arc<i32>{});
        info("%", Arc<i32>{5});

        info("%", Thread::Atomic{3});

        info("%", Vecs{Vec<i32>{1, 2}, Vec<u32>{3u, 4u}});
        info("% %", format_typename<Array<Vec<i32>, 2>>(),
             Array<Vec<i32>, 2>{Vec<i32>{1, 2}, Vec<i32>{3, 4}});

        {
            Rc<i32> r{5};
            Rc<i32> r2 = r.dup();
            assert(r2.references() == 2);
            info("%", r2);
        }
        {
            Arc<i32> r{5};
            Arc<i32> r2 = r.dup();
            assert(r2.references() == 2);
            info("%", r2);
        }

        using namespace Math;

        info("%", Vec2{1.0f, 2.0f});
        info("%", Vec3{1.0f, 2.0f, 3.0f});
        info("%", Vec4{1.0f, 2.0f, 3.0f, 4.0f});
        info("%", VecN<5>{1.0f, 2.0f, 3.0f, 4.0f, 5.0f});
        info("%", Vec2i{1, 2});
        info("%", Vec3i{1, 2, 3});
        info("%", Vec4i{1, 2, 3, 4});
        info("%", VecNi<5>{1, 2, 3, 4, 5});
        info("%", Vec2u{1u, 2u});
        info("%", Vec3u{1u, 2u, 3u});
        info("%", Vec4u{1u, 2u, 3u, 4u});
        info("%", VecNu<5>{1u, 2u, 3u, 4u, 5u});

        info("%", Quat{1.0f, 2.0f, 3.0f, 4.0f});
        info("%", BBox{Vec3{1.0f, 2.0f, 3.0f}, Vec3{4.0f, 5.0f, 6.0f}});
        info("%", Mat4{1.0f, 2.0f, 3.0f, 4.0f,    //
                       5.0f, 6.0f, 7.0f, 8.0f,    //
                       9.0f, 10.0f, 11.0f, 12.0f, //
                       13.0f, 14.0f, 15.0f, 16.0f});
    }();

    [] { // Thread
        auto value = Thread::spawn([]() {
            info("Hello from thread");
            return Vec<i32>{2, 3};
        });

        auto v = Thread::spawn([]() { info("Hello from thread2"); });

        auto gone0 = Thread::spawn([]() { info("Hello from thread4"); });
        (void)gone0;

        auto gone = Thread::Thread{[]() { info("Hello from thread3"); }};
        (void)gone;

        info("Thread 1 returned %", value->block());
        v->block();
    }();

    [] { Files::read("unknown"_v); }();

    [] {
        Net::Address addr{"127.0.0.1"_v, 25565};
        Net::Udp udp;

        udp.bind(addr);
        Net::Packet packet;
        u64 i = 0;
        for(char c : "Hello"_v) {
            packet[i++] = c;
        }
        udp.send(addr, packet, 5);

        auto data = udp.recv(packet);
        assert(data);
        assert(data->length == 5);
        info("%", String_View{packet.data(), data->length});
    }();

    // Coroutines
    [] {
        int wtf_clang_format = 0;
        (void)wtf_clang_format;

        {
            auto co = []() -> Async::Task<void> {
                info("Hello from coroutine 1");
                co_return;
            };
            Async::Task<void> task = co();
            assert(task.done());
            task.block();
        }
        {
            auto co = []() -> Async::Task<void> {
                co_await Async::Continue{};
                info("Hello from coroutine 2");
                co_return;
            };
            Async::Task<void> task = co();
            assert(task.done());
            task.block();
        }
        {
            auto co = []() -> Async::Task<void> {
                co_await Async::Suspend{};
                info("Hello from coroutine 3");
                co_return;
            };
            Async::Task<void> task = co();
            assert(!task.done());
            task.resume();
            assert(task.done());
            task.block();
        }
        {
            auto co1 = []() -> Async::Task<i32> {
                info("Hello from coroutine 4");
                co_return 1;
            };
            auto co2 = [&co1]() -> Async::Task<i32> {
                info("Hello from coroutine 5");
                i32 i = co_await co1();
                info("Coroutine 5 got %", i);
                co_return i;
            };

            Async::Task<i32> task = co2();
            assert(task.done());
            assert(task.block() == 1);
        }
        {
            auto co1 = []() -> Async::Task<i32> {
                info("Hello from coroutine 6");
                co_await Async::Suspend{};
                co_return 1;
            }();
            auto co2 = [&co1]() -> Async::Task<i32> {
                info("Hello from coroutine 7");
                i32 i = co_await co1;
                info("Coroutine 7 got %", i);
                co_return i;
            };

            Async::Task<i32> task = co2();
            assert(!task.done());
            co1.resume();
            assert(task.done());
            assert(task.block() == 1);
        }
        {
            auto co1 = []() -> Async::Task<i32> {
                info("Hello from coroutine 8");
                co_await Async::Suspend{};
                co_await Async::Suspend{};
                co_return 1;
            };

            auto job = co1();

            auto co2 = [&job]() -> Async::Task<i32> {
                info("Hello from coroutine 9");
                i32 i = co_await job;
                info("Coroutine 9 got %", i);
                co_return i;
            };

            Async::Task<i32> task = co2();
            assert(!task.done());
            job.resume();
            job.resume();
            assert(task.done());
            assert(task.block() == 1);
        }
    }();

    // Threads
    [] {
        Vec<Thread::Future<void>> tasks;
        for(u64 i = 0; i < Thread::hardware_threads(); i++) {
            tasks.push(Thread::spawn([]() { info("Hello from thread"); }));
        }

        for(auto& task : tasks) {
            task->block();
        }
    }();

    // Thread pool
    [] {
        Thread::Pool pool;

        for(u64 i = 0; i < 100; i++) {
            assert(lots_of_jobs(pool, 10).block() == 1024);
        }
    }();

    [] {
        Thread::Pool pool;

        {
            auto job = [&pool]() -> Async::Task<i32> {
                co_await pool.suspend();
                info("Hello from coroutine 1 on thread pool");
                co_return 1;
            }();

            info("Job returned %", job.block());
        }
        {
            auto job = [&pool]() -> Async::Task<i32> {
                co_await pool.suspend();
                info("Hello from coroutine 2 on thread pool");
                co_return 1;
            };
            job();
            job();
            job();
        }
        {
            auto job = [&pool_ = pool]() -> Async::Task<i32> {
                // pool will be gone after the first suspend, so we need to copy it
                Thread::Pool<>& pool = pool_;
                co_await pool.suspend();
                info("Hello from coroutine 4.1 on thread pool");
                co_await pool.suspend();
                info("Hello from coroutine 4.2 on thread pool");
                co_await pool.suspend();
                info("Hello from coroutine 4.3 on thread pool");
                co_return 1;
            };
            job().block();
        }
        {
            auto job = [&pool_ = pool]() -> Async::Task<i32> {
                // pool will be gone after the first suspend, so we need to copy it
                Thread::Pool<>& pool = pool_;
                co_await pool.suspend();
                info("Hello from coroutine 3.1 on thread pool");
                co_await pool.suspend();
                info("Hello from coroutine 3.2 on thread pool");
                co_await pool.suspend();
                info("Hello from coroutine 3.3 on thread pool");
                co_return 1;
            };
            job();
            job();
            job();
            // These are OK to drop because the promises are refcounted and
            // no job can deadlock itself
        }
    }();

    // Scheduling
    [] {
        Thread::Pool pool;

        {
            auto job = [&pool_ = pool](i32 ms) -> Async::Task<i32> {
                auto& pool = pool_;
                info("5.2 begin %", ms);
                co_await pool.suspend();
                info("5.2 on thread %", ms);
                Thread::sleep(ms);
                info("5.2 done %", ms);
                co_return 1;
            };
            auto job2 = [&pool_ = pool, &job_ = job]() -> Async::Task<i32> {
                auto& pool = pool_;
                auto& job = job_;
                info("5.1 begin");
                co_await pool.suspend();
                info("5.1 on thread");
                // TODO: to be able to block on a job in a coroutine, the scheduler
                // needs to support CPU affinity again.
                info("5.1: co_await 1ms job");
                i32 i = co_await job(1);
                info("5.1: launch 0s job");
                auto wait = job(0); // should run on another thread, we don't yield until the next
                info("5.1: co_await 100ms job");
                i32 j = co_await job(100);
                // same thread should pick up the job as we wait immediately
                // continues on the same thread via continuation
                info("5.1: co_await 0s job");
                i32 k = co_await wait;
                // does not wait or use continuation because already done
                info("5.1 done: % % %", i, j, k);
                co_return i + j + k;
            };

            assert(job2().block() == 3);
            // cannot start and drop another job2 because pending continuations
            // are leaked
        }
        {
            Function<Async::Task<void>(u64)> lots_of_jobs =
                [&pool_ = pool, &lots_of_jobs_ = lots_of_jobs](u64 depth) -> Async::Task<void> {
                auto& pool = pool_;
                auto& lots_of_jobs = lots_of_jobs_;
                if(depth == 0) {
                    co_return;
                }
                co_await pool.suspend();
                auto job0 = lots_of_jobs(depth - 1);
                auto job1 = lots_of_jobs(depth - 1);
                co_await job0;
                co_await job1;
            };

            lots_of_jobs(14).block();
        }
    }();

    // Async IO
    [] {
        Thread::Pool pool;
        {
            auto job = [&pool_ = pool]() -> Async::Task<void> {
                auto& pool = pool_;
                info("coWaiting 100ms.");
                co_await AsyncIO::wait(pool, 100);
                info("coWaited 100ms.");
                co_return;
            };

            job().block();
            info("Waited 100ms.");
        }
    }();
    [] {
        Thread::Pool pool;
        {
            Vec<u8> large(4096 + 1);
            large.resize(4096 + 1);
            AsyncIO::write(pool, "large_file"_v, Slice<u8>{large}).block();
        }
        {
            auto file = AsyncIO::read(pool, "large_file"_v).block();
            info("Read file: %", file->length());
        }
        {
            auto job = [&pool_ = pool]() -> Async::Task<Vec<u8, AsyncIO::Alloc>> {
                auto& pool = pool_;
                info("coReading file.");
                auto file = co_await AsyncIO::read(pool, "large_file"_v);
                info("coRead file: %", file->length());
                co_return std::move(*file); // dies access violation resuming coroutine
            };

            auto file = job().block();
            info("Read file: %", file.length());
        }
    }();

    Profile::end_frame();
    Profile::end_thread();
    Profile::finalize();
    Mregion::destroy();

    return 0;
}