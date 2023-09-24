
#include <functional>

#include "rpp/async.h"
#include "rpp/base.h"
#include "rpp/files.h"
#include "rpp/net.h"
#include "rpp/pool.h"
#include "rpp/range_allocator.h"
#include "rpp/thread.h"

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
struct Reflect<Ints> {
    using T = Ints;
    static constexpr Literal name = "Ints";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(i), FIELD(u)>;
    static_assert(Record<T>);
};
template<>
struct Reflect<Vecs> {
    using T = Vecs;
    static constexpr Literal name = "Vecs";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(i), FIELD(u)>;
    static_assert(Record<T>);
};
} // namespace rpp

i32 main() {

    Profile::track_alloc_stats();
    Thread::set_priority(Thread::Priority::high);
    Profile::start_thread();

    Profile::begin_frame();

    // Alloc0
    {
        Prof_Scope("Alloc0");

        using A = Mallocator<"Test">;
        void* ptr = A::alloc(100);
        A::free(ptr);
    }

    // Ref0/1
    {
        Prof_Scope("Ref0/1");

        i32 i = 5;
        Ref<i32> r{i};

        assert(*r == 5);
        i = 10;

        Ref<Ref<i32>> rr{r};
        assert(**rr == 10);
    }

    // String0
    {
        Prof_Scope("String0");

        String_View sv = "Hello World"_v;
        String s = sv.string();

        s[0] = 'h';

        for(char c : sv) {
            info("%", c);
        }
        for(char c : s) {
            info("%", c);
        }

        String_View sv2 = sv;
        String_View sv3 = std::move(sv2);
        String_View sv4 = s.view();
        String s2 = sv3.string();

        String s3 = s.clone();
        String s4 = std::move(s3);

        (void)s4;
        (void)sv4;
    }

    // Thread0
    {
        Prof_Scope("Thread0");

        Thread::Mutex mut;
        mut.lock();
        mut.unlock();
        { Thread::Lock lock{mut}; }
    }

    // Log
    {
        Prof_Scope("Log");

        info("Info");
        warn("Warn");
    }

    using M = Thread::Mutex;
    using SV = String_View;

    // Pair
    {
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
    }

    // Storage
    {
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
    }

    // Opt
    {
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
    }

    // Array
    {
        Prof_Scope("Array");

        Array<i32, 1> a;
        a[0] = 5;

        Array<i32, 4> b{1, 2, 3, 4};
        Array<i32, 4> c = b;

        assert(c[2] == 3);

        for(i32 i : c) {
            info("%", i);
        }
        for(i32& i : c) {
            info("%", i);
        }

        const Array<i32, 4>& cc = c;
        for(i32 i : cc) {
            info("%", i);
        }
        for(const i32& i : cc) {
            info("%", i);
        }

        Array<i32, 4> d = std::move(c);
        assert(d[2] == 3);

        Array<SV, 3> e{"Hello"_v, "World"_v, "!"_v};

        Array<SV, 3> f = e.clone();

        Array<M, 2> m;

        (void)f;
    }

    // Vec
    {
        Prof_Scope("Vec");

        Vec<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);

        assert(v.length() == 3);

        for(i32 i : v) {
            info("%", i);
        }
        for(i32& i : v) {
            info("%", i);
        }

        const auto& constv = v;
        for(i32 i : constv) {
            info("%", i);
        }
        for(const i32& i : constv) {
            info("%", i);
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

        Slice<i32> s6 = {1, 2, 3};
        assert(s6.length() == 3);

        Slice<SV> s7 = {"Hello"_v, "World"_v};
        assert(s7.length() == 2);

        Slice<M> smm;
        (void)smm;
        Slice<M> sm = {M{}};

        Slice<M> m2{};

        Vec<std::function<void()>> vf;
        for(i32 i = 0; i < 10; i++) {
            vf.push([]() { info("Hello"); });
        }

        (void)m2;
        (void)s3;
        (void)s5;
    }

    // Box
    {
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
    }

    // Stack
    {
        Prof_Scope("Stack");

        Stack<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);

        assert(v.length() == 3);

        for(i32 i : v) {
            info("%", i);
        }
        for(i32& i : v) {
            info("%", i);
        }

        const auto& constv = v;
        for(i32 i : constv) {
            info("%", i);
        }
        for(const i32& i : constv) {
            info("%", i);
        }

        v.pop();
        assert(v.length() == 2);

        Stack<i32> v2 = v.clone();
        Stack<i32> v3 = std::move(v2);

        assert(v3.length() == 2);

        Stack<SV> sv{"Hello"_v, "World"_v};
        Stack<SV> sv2 = sv.clone();
        Stack<SV> sv3 = std::move(sv2);

        assert(sv3.length() == 2);

        Stack<std::function<void()>> vf;
        for(i32 i = 0; i < 10; i++) {
            vf.push([]() { info("Hello"); });
        }
    }

    // Queue
    {
        Prof_Scope("Queue");

        {
            Queue<String<>> sv{"Hello"_v.string(), "World"_v.string()};
            sv.pop();
        }

        Queue<i32> v;
        v.push(1);
        v.push(2);
        v.push(3);

        for(i32 i : v) {
            info("%", i);
        }
        for(i32& i : v) {
            info("%", i);
        }

        const auto& constv = v;
        for(i32 i : constv) {
            info("%", i);
        }
        for(const i32& i : constv) {
            info("%", i);
        }

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

        Queue<std::function<void()>> vf;
        for(i32 i = 0; i < 10; i++) {
            vf.push([]() { info("Hello"); });
        }
    }

    // Heap
    {
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

        for(i32 i : v) {
            info("%", i);
        }
        for(i32& i : v) {
            info("%", i);
        }

        const auto& constv = v;
        for(i32 i : constv) {
            info("%", i);
        }
        for(const i32& i : constv) {
            info("%", i);
        }

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
            std::function<void()> f;
            bool operator<(const F& other) const {
                return true;
            }
        };

        Heap<F> vf;
        for(i32 i = 0; i < 10; i++) {
            vf.push({[]() { info("Hello"); }});
        }
    }

    // Map
    {
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

        Map<i32, std::function<void()>> ff;
        for(i32 i = 0; i < 40; i++) {
            ff.insert(i, []() { info("Hello"); });
        }
    }

    // Rc
    {
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
    }

    // Arc
    {
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
    }

    // Format
    {
        info("%%");

        info("%", true);
        info("%", false);

        i32 array[2] = {1, 2};
        info("%", array);

        info("%", Ints{10, 2});
        // Ints{i : 10, u : 2}

        info("%", Kind::enum_);

        info("%", "Hello"_v);
        info("%", "Hello"_v.string());

        info("%", Ref<i32>{});

        i32 i = 0;
        info("%", Ref{i});

        info("%", null);
        info("%", &i);

        info("%", Box<i32>{});
        info("%", Box<i32>{i});

        info("%", Pair{23, 13.0f});

        info("%", Storage<Ints>{});

        info("%", Opt<i32>{});
        info("%", Opt<i32>{5});

        info("%", Array<i32, 2>{1, 2});
        info("%", Array<i32, 2>{});

        info("%", Vec<i32>{1, 2});
        info("%", Vec<i32>{});

        info("%", Stack<i32>{1, 2});
        info("%", Stack<i32>{});

        info("%", Queue<i32>{1, 2});
        info("%", Queue<i32>{});

        info("%", Heap<i32>{1, 2});
        info("%", Heap<i32>{});

        info("%", Map<i32, i32>{Pair{1, 2}, Pair{3, 4}});
        info("%", Map<i32, i32>{});

        info("%", Rc<i32>{});
        info("%", Rc<i32>{5});

        info("%", Arc<i32>{});
        info("%", Arc<i32>{5});

        info("%", Thread::Atomic{3});

        info("%", Vecs{Vec<i32>{1, 2}, Vec<u32>{3u, 4u}});
        info("%", Array<Vec<i32>, 2>{Vec<i32>{1, 2}, Vec<i32>{3, 4}});

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
    }

    { // Thread
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
    }

    { Files::read("unknown"_v); }

    {
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
    }

    // Coroutines
    {
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
            auto co = []() -> Async::Task<void> {
                co_await Async::Suspend{};
                info("Hello from coroutine 3");
                co_return;
            };
            Async::Task<void> task = co();
            assert(!task.done());
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
    }

    { // Thread pool
        Thread::Pool pool;

        Vec<Thread::Future<void>> tasks;
        for(u64 i = 0; i < Thread::hardware_threads(); i++) {
            tasks.push(
                pool.single(Thread::Priority::normal, []() { info("Hello from thread pool"); }));
        }

        for(auto& task : tasks) {
            task->block();
        }

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
            auto job = [&pool]() -> Async::Task<i32> {
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
            auto job = [&pool]() -> Async::Task<i32> {
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
    }
    {
        Thread::Pool pool;
        {
            auto job = [&pool](i32 ms) -> Async::Task<i32> {
                info("5.2 begin %", ms);
                co_await pool.suspend();
                info("5.2 on thread %", ms);
                Thread::sleep(ms);
                info("5.2 done %", ms);
                co_return 1;
            };
            auto job2 = [&pool, &job]() -> Async::Task<i32> {
                info("5.1 begin");
                co_await pool.suspend();
                info("5.1 on thread");
                info("5.1: wait on 0.99s job");
                i32 i = job(99).block(); // has to run on another thread, blocks this thread
                // continue on same thread, wait does not swap out
                info("5.1: co_await 1ms job");
                auto j = co_await job(1); // likely runs on this thread as we yield immediately
                // should continue on same thread because we had time to install the continuation
                info("5.1: launch 0s job");
                auto wait =
                    job(0); // should run on another thread, we don't yield until the next await
                info("5.1: co_await 100ms job");
                i32 k =
                    co_await job(100); // same thread should pick up the job as we wait immediately
                // continues on the same thread via continuation
                info("5.1: co_await 0s job");
                i32 l = co_await wait;
                // does not wait or use continuation because already done
                info("5.1 done: % % % %", i, j, k, l);
                co_return i + j + k + l;
            };

            assert(job2().block() == 4);
            // cannot start and drop another job2 because it blocks on another job -
            // if all but one thread shut down it deadlocks itself
        }
    }

    Profile::end_frame();
    Profile::end_thread();

    return 0;
}
