
#include <rpp/base.h>
#include <rpp/files.h>
#include <rpp/log_callback.h>

using namespace rpp;

struct Test {
    explicit Test(String_View name) : name(name) {
        token = Log::subscribe(
            [&](Log::Level lvl, Thread::Id, Log::Time, Log::Location, String_View msg) {
                auto m = format<Mhidden>("[%] %\n"_v, lvl, msg);
                for(u8 c : m) {
                    result.push(c);
                }
            });
    }
    ~Test() {
        Log::unsubscribe(token);

        auto expect = name.append<Mdefault>(".expect"_v);
        expected = move(*Files::read(expect.view()));

        bool differs = false;
        if(result.length() != expected.length()) {
            differs = true;
        } else {
            for(u64 i = 0; i < result.length(); i++) {
                if(result[i] != expected[i]) {
                    differs = true;
                    break;
                }
            }
        }

        if(differs) {
            auto corrected = name.append<Mdefault>(".corrected"_v);
            static_cast<void>(Files::write(corrected.view(), result.slice()));
            Libc::exit(1);
        }
    }

    String_View name;
    Vec<u8, Mhidden> result;
    Vec<u8, Files::Alloc> expected;
    Log::Token token;
};
