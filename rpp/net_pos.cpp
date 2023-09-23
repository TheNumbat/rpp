
#include "net.h"

#include <arpa/inet.h>
#include <unistd.h>

namespace rpp::Net {

Address::Address(String_View address, u16 port) {

    sockaddr_ = {};
    sockaddr_.sin_family = AF_INET;
    sockaddr_.sin_port = htons(port);

    if(inet_pton(AF_INET, reinterpret_cast<const char*>(address.data()),
                 &sockaddr_.sin_addr.s_addr) != 1) {
        die("Failed to create address: %", Log::sys_error());
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
    fd = fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0) {
        die("Failed to open socket: %", Log::sys_error());
    }
}

Udp::~Udp() {
    if(fd != -1) {
        close(fd);
    }
}

Udp::Udp(Udp&& src) {
    fd = src.fd;
    src.fd = -1;
}

Udp& Udp::operator=(Udp&& src) {
    fd = src.fd;
    src.fd = -1;
    return *this;
}

void Udp::bind(Address address) {
    if(::bind(fd, reinterpret_cast<const sockaddr*>(&address.sockaddr()), sizeof(sockaddr_in)) <
       0) {
        die("Failed to bind socket: %", Log::sys_error());
    }
}

Opt<Udp::Data> Udp::recv(Packet& in) {

    sockaddr_in src;
    socklen_t src_len = sizeof(src);
    i64 ret = ::recvfrom(fd, in.begin(), in.capacity, MSG_DONTWAIT | MSG_TRUNC,
                         reinterpret_cast<sockaddr*>(&src), &src_len);

    if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return Opt<Data>{};
    }

    return Opt{Data{static_cast<u64>(ret), Address{src}}};
}

u64 Udp::send(Address address, const Packet& out, u64 length) {

    i64 ret = sendto(fd, out.data(), length, MSG_CONFIRM,
                     reinterpret_cast<const sockaddr*>(&address.sockaddr()), sizeof(sockaddr_in));
    if(ret == -1) {
        die("Failed send packet: %", Log::sys_error());
    }
    return ret;
}

} // namespace rpp::Net
