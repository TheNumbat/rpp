
#include "test.h"

#include <variant.h>

i32 main() {
    Test test{"variant"_v};
    {
        {
            Variant<i8, i16, i32, i64, u8, u16, u32, u64, f32, f64> v{1.0f};
            v.match([](auto i) { info("variant has %", i); });
        }
        {
            Variant<String<>> v{"Hello"_v.string()};
            auto s = move(v);
        }

        static_assert(alignof(i32) == 4);
        Variant<i32> v{1};
        info("sizeof variant %", sizeof(v));

        v.match([](i32 i) { info("variant has %", i); });
        v.match([](i32& i) { info("variant has %", i); });

        Variant<i32, String_View> v2{"Hello"_v};

        v2.match(Overload{[](i32 i) { info("variant has int %", i); },
                          [](String_View s) { info("variant has string view %", s); }});

        String<> i = v2.match(Overload{[](i32 i) {
                                           info("variant has int %", i);
                                           return "int"_v.string();
                                       },
                                       [](String_View s) {
                                           info("variant has string view %", s);
                                           return "string"_v.string();
                                       }});

        info("returned % from match", i);

        Variant<i32, String_View> v3 = v2.clone();
        assert(v3.index() == 1);
        Variant<i32, String_View> v4 = move(v3);
        assert(v4.index() == 1);

        { //
            Variant<i32, String<>> v5{"Hello"_v.string()};
            Variant<i32, String<>> v6 = v5.clone();
            Variant<i32, String<>> v7 = move(v6);

            Variant<i32, String<>> v8{1};
            Variant<i32, String<>> v9 = v8.clone();
            Variant<i32, String<>> v10 = move(v9);
        }

        Variant<Named<"1", i32>, Named<"2", i32>> v11{Named<"1", i32>{1}};

        v11.match([](const auto& i) {
            info("variant has %: %", String_View{Decay<decltype(i)>::name}, i);
        });
    }

    return 0;
}
