
#include "test.h"

i32 main() {
    Test test{"empty"_v};
    Trace("String0") {
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
        String_View sv3 = move(sv2);
        String_View sv4 = s.view();
        String s2 = sv3.string();

        String s3 = s.clone();
        String s4 = move(s3);

        (void)s4;
        (void)sv4;
    }

    return 0;
}
