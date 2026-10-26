
#include "test.h"

#include <rpp/net.h>

i32 main() {
    Test test{"net"_v};
    {
        Net::Address addr{"127.0.0.1"_v, 25565};
        Net::Udp udp;

        udp.bind(addr);
        auto& self = udp;
        udp = move(self);
        Net::Packet packet;
        u64 i = 0;
        for(char c : "Hello"_v) {
            packet[i++] = c;
        }
        static_cast<void>(udp.send(addr, packet, 5));

        Thread::sleep(100);

        auto data = udp.recv(packet);
        assert(data.ok());
        assert(data->length == 5);
        info("%", String_View{packet.data(), data->length});
    }
    {
        Net::Address addr{"127.0.0.1"_v, 25565};
        Net::Udp dst, src;
        dst.bind(addr);
        dst = move(src);

        Net::Udp rebound;
        rebound.bind(addr);
        src = move(rebound);
    }
    {
        Net::Udp rebound;
        rebound.bind(Net::Address{"127.0.0.1"_v, 25565});
    }
    return 0;
}
