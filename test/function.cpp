
#include "test.h"

#include <function.h>

i32 main() {
    Test test{"function"_v};
    {
        Function<void()> f{
            []() { info("Hello function"); },
        };

        f();

        Function<void()> f2 = move(f);

        f2();

        String<> test = "Hello"_v.string();

        Function<void()> f3{
            [&test]() { info("Hello function 2: %", test); },
        };

        f3();

        Function<void()> f4{
            [test = move(test)]() { info("Hello function 2: %", test); },
        };

        f4();

        Function<void()> f5 = move(f4);

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
        f8(s, s, s, move(s2));

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
    }
    return 0;
}
