
#pragma once

#include "base.h"

#ifdef OS_WINDOWS
#define Arc Arc__w32
#include <windows.h>
#undef Arc
#include <ws2def.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

namespace rpp::Net {

constexpr u16 default_port = 6969;
constexpr u64 min_transmissible_unit = 1472;

using Packet = Array<u8, min_transmissible_unit>;

struct Address {

    Address() = default;

    explicit Address(sockaddr_in sockaddr);
    explicit Address(u16 port);
    explicit Address(String_View address, u16 port);

    const sockaddr_in& sockaddr() const {
        return sockaddr_;
    }

private:
    sockaddr_in sockaddr_;
};

struct Udp {
    struct Data {
        u64 length;
        Address from;
    };

    Udp();
    ~Udp();

    Udp(const Udp& src) = delete;
    Udp& operator=(const Udp& src) = delete;

    Udp(Udp&& src);
    Udp& operator=(Udp&& src);

    void bind(Address address);
    u64 send(Address address, const Packet& out, u64 length);
    Opt<Data> recv(Packet& in);

private:
#ifdef OS_WINDOWS
    u64 socket;
#else
    i32 fd;
#endif
};

} // namespace rpp::Net
