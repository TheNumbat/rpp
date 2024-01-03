
#include "test.h"

#include <net.h>

i32 main() {

    Profile::start_thread();
    Profile::begin_frame();

    {
        Test test{"net"_v};
        {
            Net::Address addr{"127.0.0.1"_v, 25565};
            Net::Udp udp;

            udp.bind(addr);
            Net::Packet packet;
            u64 i = 0;
            for(char c : "Hello"_v) {
                packet[i++] = c;
            }
            static_cast<void>(udp.send(addr, packet, 5));

            auto data = udp.recv(packet);
            assert(data);
            assert(data->length == 5);
            info("%", String_View{packet.data(), data->length});
        }
    }

    Profile::end_frame();
    Profile::finalize();

    return 0;
}
