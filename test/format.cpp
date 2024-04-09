
#include "test.h"

#include <rpp/function.h>
#include <rpp/heap.h>
#include <rpp/rc.h>
#include <rpp/stack.h>
#include <rpp/tuple.h>
#include <rpp/variant.h>
#include <rpp/vmath.h>

struct Ints {
    i32 i;
    u16 u;
};

struct Vecs {
    Vec<i32> i;
    Vec<u32> u;
};

RPP_RECORD(Ints, RPP_FIELD(i), RPP_FIELD(u));
RPP_RECORD(Vecs, RPP_FIELD(i), RPP_FIELD(u));

i32 main() {
    Test test{"format"_v};
    {
        info("%%");

        info("%", true);
        info("%", false);

        i32 array[2] = {1, 2};
        info("% %", format_typename<decltype(array)>(), array);

        info("% %", format_typename<Ints>(), Ints{10, 2});
        // Ints{i : 10, u : 2}

        info("% %", format_typename<Reflect::Kind>(), Reflect::Kind::enum_);

        info("%", "Hello"_v);
        info("%", "Hello"_v.string());

        info("% %", format_typename<Ref<i32>>(), Ref<i32>{});

        i32 i = 0;
        info("%", Ref{i});

        info("%", null);

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

        info("% %", format_typename<Slice<const i32>>(), Slice<const i32>{1, 2});
        info("%", Slice<i32>{});

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
    }
    return 0;
}
