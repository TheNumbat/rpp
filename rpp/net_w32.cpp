
#include "net.h"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace rpp::Net {

static String_View wsa_error() {

    constexpr u64 buffer_size = 64;
    char buffer[buffer_size] = {};

    i32 err = WSAGetLastError();

    u32 size = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, null, err,
                              LANG_USER_DEFAULT, buffer, buffer_size, null);
    if(!size) {
        std::snprintf(buffer, buffer_size, "WinSock Error: %d", err);
        return String_View{buffer};
    }

    return String_View{buffer};
}

struct WSA_Startup {
    WSA_Startup() {
        WSADATA wsa;
        if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            die("Failed to startup winsock: %", wsa_error());
        }
    }
};

static WSA_Startup g_wsa_startup;

Address::Address(const String_View& address, u16 port) {

    sockaddr_ = {};
    sockaddr_.sin_family = AF_INET;
    sockaddr_.sin_port = htons(port);

    if(inet_pton(AF_INET, reinterpret_cast<const char*>(address.data()),
                 &sockaddr_.sin_addr.s_addr) != 1) {
        die("Failed to create address: %", wsa_error());
    }
}

Address::Address(sockaddr_in sockaddr) : sockaddr_(sockaddr) {
}

Address::Address(u16 port) {
    sockaddr_ = {};
    sockaddr_.sin_family = AF_INET;
    sockaddr_.sin_port = htons(port);
    sockaddr_.sin_addr.s_addr = INADDR_ANY;
}

Udp::Udp() {

    socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket == INVALID_SOCKET) {
        die("Failed to open socket: %", wsa_error());
    }

    u_long imode = 1;
    if(ioctlsocket(socket, FIONBIO, &imode) != NO_ERROR) {
        die("Failed to set socket nonblocked: %", wsa_error());
    }
}

Udp::~Udp() {
    if(socket != INVALID_SOCKET) {
        closesocket(socket);
    }
    socket = INVALID_SOCKET;
}

Udp::Udp(Udp&& src) {
    socket = src.socket;
    src.socket = INVALID_SOCKET;
}

Udp& Udp::operator=(Udp&& src) {
    socket = src.socket;
    src.socket = INVALID_SOCKET;
    return *this;
}

void Udp::bind(Address address) {

    if(::bind(socket, (SOCKADDR*)&address.sockaddr(), sizeof(sockaddr_in)) == SOCKET_ERROR) {
        die("Failed to bind socket: %", wsa_error());
    }
}

Opt<Udp::Data> Udp::recv(Packet& in) {

    sockaddr_in src;

    i32 src_len = sizeof(src);
    i32 ret = recvfrom(socket, reinterpret_cast<char*>(in.data()), static_cast<i32>(in.length()), 0,
                       reinterpret_cast<SOCKADDR*>(&src), &src_len);
    if(ret == SOCKET_ERROR) {
        return {};
    }

    return Opt{Data{static_cast<u64>(ret), Address{src}}};
}

u64 Udp::send(Address address, const Packet& out, u64 length) {

    i32 ret = sendto(socket, reinterpret_cast<const char*>(out.data()), static_cast<i32>(length), 0,
                     reinterpret_cast<const SOCKADDR*>(&address.sockaddr()), sizeof(sockaddr_in));
    if(ret == SOCKET_ERROR) {
        warn("Failed send packet: %", wsa_error());
    }
    return ret;
}

} // namespace rpp::Net
