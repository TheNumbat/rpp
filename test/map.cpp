
#include "test.h"

i32 main() {

    Profile::start_thread();
    Profile::begin_frame();

    {
        Test test{"map"_v};
        {
            Prof_Scope("Map");
            {
                Map<String_View, i32> int_map{Pair{"foo"_v, 0}, Pair{"bar"_v, 1}};

                int_map.insert("baz"_v, 2);
                int_map.erase("bar"_v);

                for(auto& [key, value] : int_map) info("%: %", key, value);
            }

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
            Map<i32, i32> v3 = move(v2);

            assert(v3.length() == 2);

            Map<i32, String_View> i_sv{Pair{1, "Hello"_v}, Pair{2, "World"_v}};
            Map<String_View, i32> sv_i{Pair{"Hello"_v, 1}, Pair{"World"_v, 2}};

            Map<String_View, String_View> sv{Pair{"Hello"_v, "World"_v}};

            Map<String_View, String_View> sv2 = sv.clone();
            Map<String_View, String_View> sv3 = move(sv2);

            assert(sv3.length() == 1);

            Map<i32, Function<void()>> ff;
            for(i32 i = 0; i < 40; i++) {
                ff.insert(i, []() { info("Hello"); });
            }
        }
    }

    Profile::end_frame();
    Profile::finalize();

    return 0;
}
