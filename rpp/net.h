
#pragma once

#include "base.h"

#if defined RPP_OS_LINUX || defined RPP_OS_MACOS 
#include <netinet/in.h>
#include <sys/socket.h>
#endif

namespace rpp::Net {

constexpr u16 default_port = 6969;
constexpr u64 min_transmissible_unit = 1472;

using Packet = Array<u8, min_transmissible_unit>;

struct Address {

    Address() = default;

    explicit Address(u16 port) noexcept;
    explicit Address(String_View address, u16 port) noexcept;

private:
#ifdef RPP_OS_WINDOWS
    alignas(4) u8 sockaddr_storage[16];
#else
    sockaddr_in sockaddr_;
#endif

    friend struct Udp;
};

struct Udp {
    struct Data {
        u64 length;
        Address from;
    };

    Udp() noexcept;
    ~Udp() noexcept;

    Udp(const Udp& src) noexcept = delete;
    Udp& operator=(const Udp& src) noexcept = delete;

    Udp(Udp&& src) noexcept;
    Udp& operator=(Udp&& src) noexcept;

    void bind(Address address) noexcept;
    [[nodiscard]] u64 send(Address address, const Packet& out, u64 length) noexcept;
    [[nodiscard]] Opt<Data> recv(Packet& in) noexcept;

private:
#ifdef RPP_OS_WINDOWS
    u64 socket;
#else
    i32 fd;
#endif
};

} // namespace rpp::Net
