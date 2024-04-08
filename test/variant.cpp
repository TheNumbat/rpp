
#include "test.h"

#include <rpp/variant.h>

struct InstCnt {
    InstCnt() {
        ++cnt;
    }
    ~InstCnt() {
        --cnt;
    }
    InstCnt(const InstCnt&) {
        ++cnt;
    }
    InstCnt(InstCnt&&) {
        ++cnt;
    }
    InstCnt& operator=(const InstCnt&) = delete;
    InstCnt& operator=(InstCnt&&) = delete;

    inline static i32 cnt = 0;
};

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
    {
        {
            Variant<InstCnt> c1{InstCnt{}};
            assert(InstCnt::cnt == 1);

            // Move assign value type.
            c1 = InstCnt{};
            assert(InstCnt::cnt == 1);
        }
        assert(InstCnt::cnt == 0);

        { // Move construct from value type.
            Variant<InstCnt> c1(InstCnt{});
            {
                Variant<InstCnt> c2(InstCnt{});
                assert(InstCnt::cnt == 2);

                // Move construct from Variant type, moves inner value.
                c1 = move(c2);
                assert(InstCnt::cnt == 2);
            }
            assert(InstCnt::cnt == 1);
        }
        assert(InstCnt::cnt == 0);

        // Move construct from Variant type.
        {
            Variant<InstCnt> c1(InstCnt{});
            assert(InstCnt::cnt == 1);
            {
                Variant<InstCnt> c2(move(c1));
                assert(InstCnt::cnt == 2);
            }
            assert(InstCnt::cnt == 1);
        }
        assert(InstCnt::cnt == 0);

        // Clone and move assign Variant type.
        {
            Variant<InstCnt> c1(InstCnt{});
            assert(InstCnt::cnt == 1);
            {
                Variant<InstCnt> c2 = c1.clone();
                assert(InstCnt::cnt == 2);
            }
            assert(InstCnt::cnt == 1);
        }
        assert(InstCnt::cnt == 0);
    }
    return 0;
}
