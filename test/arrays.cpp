
#include "test.h"

#include <heap.h>
#include <stack.h>

i32 main() {

    Profile::start_thread();
    Profile::begin_frame();

    {
        Test test{"empty"_v};
        Trace("Array") {
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

            Array<i32, 4> d = move(c);
            assert(d[2] == 3);

            Array<String_View, 3> e{"Hello"_v, "World"_v, "!"_v};

            Array<String_View, 3> f = e.clone();

            Array<Thread::Mutex, 2> m;

            (void)f;
        }
        Trace("Vec") {
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
            Vec<i32> v3 = move(v2);

            Slice<i32> sl2 = v3.slice();
            assert(sl2.length() == 2);

            assert(v3.length() == 2);

            Vec<String_View> sv{"Hello"_v, "World"_v};
            Vec<String_View> sv2 = sv.clone();
            Vec<String_View> sv3 = move(sv2);

            assert(sv3.length() == 2);

            Slice<i32> s{v3};
            assert(s.length() == 2);

            Slice<i32> s2 = s;
            Slice<i32> s3 = move(s2);

            Slice<String_View> s4{sv3};
            assert(s4.length() == 2);
            Slice<String_View> s5 = s4;

            Slice<i32> s6{1, 2, 3};
            assert(s6.length() == 3);

            Slice<String_View> s7{"Hello"_v, "World"_v};
            assert(s7.length() == 2);

            Slice<Thread::Mutex> smm;
            (void)smm;
            Slice<Thread::Mutex> sm{Thread::Mutex{}};

            Slice<Thread::Mutex> m2{};

            Vec<Function<void()>> vf;
            for(i32 i = 0; i < 10; i++) {
                vf.push([]() { info("Hello"); });
            }

            (void)m2;
            (void)s3;
            (void)s5;
        }
        Trace("Stack") {
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
            Stack<i32> v3 = move(v2);

            assert(v3.length() == 2);

            Stack<String_View> sv{"Hello"_v, "World"_v};
            Stack<String_View> sv2 = sv.clone();
            Stack<String_View> sv3 = move(sv2);

            assert(sv3.length() == 2);

            Stack<Function<void()>> vf;
            for(i32 i = 0; i < 10; i++) {
                vf.push([]() { info("Hello"); });
            }
        }
        Trace("Queue") {
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
            Queue<i32> v3 = move(v2);

            assert(v3.length() == 2);

            Queue<String_View> sv{"Hello"_v, "World"_v};
            Queue<String_View> sv2 = sv.clone();
            Queue<String_View> sv3 = move(sv2);

            assert(sv3.length() == 2);

            Queue<Function<void()>> vf;
            for(i32 i = 0; i < 10; i++) {
                vf.push([]() { info("Hello"); });
            }
        }
        Trace("Heap") {
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
            Heap<i32> v3 = move(v2);

            assert(v3.length() == 3);

            Heap<String_View> sv{"Hello"_v, "World"_v};
            Heap<String_View> sv2 = sv.clone();
            Heap<String_View> sv3 = move(sv2);

            assert(sv3.length() == 2);

            Heap<String_View> o{"a"_v, "aa"_v, "ab"_v, "bb"_v, "aab"_v};
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
        }
    }

    Profile::end_frame();
    Profile::finalize();

    return 0;
}
