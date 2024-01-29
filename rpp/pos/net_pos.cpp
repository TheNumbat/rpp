
#include "../net.h"

#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

namespace rpp::Net {

Address::Address(String_View address, u16 port) noexcept {
    sockaddr_ = {};
    sockaddr_.sin_family = AF_INET;
    sockaddr_.sin_port = htons(port);

    if(inet_pton(AF_INET, reinterpret_cast<const char*>(address.data()),
                 &sockaddr_.sin_addr.s_addr) != 1) {
        die("Failed to create address: %", Log::sys_error());
    }
}

Address::Address(u16 port) noexcept {
    sockaddr_ = {};
    sockaddr_.sin_family = AF_INET;
    sockaddr_.sin_port = htons(port);
    sockaddr_.sin_addr.s_addr = INADDR_ANY;
}

Udp::Udp() noexcept {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0) {
        die("Failed to open socket: %", Log::sys_error());
    }
}

Udp::~Udp() noexcept {
    if(fd != -1) {
        close(fd);
    }
}

Udp::Udp(Udp&& src) noexcept {
    fd = src.fd;
    src.fd = -1;
}

Udp& Udp::operator=(Udp&& src) noexcept {
    fd = src.fd;
    src.fd = -1;
    return *this;
}

void Udp::bind(Address address) noexcept {
    if(::bind(fd, reinterpret_cast<const sockaddr*>(&address.sockaddr_), sizeof(sockaddr_in)) < 0) {
        die("Failed to bind socket: %", Log::sys_error());
    }
}

[[nodiscard]] Opt<Udp::Data> Udp::recv(Packet& in) noexcept {

    Address src;
    socklen_t src_len = sizeof(src.sockaddr_);

    i64 ret = ::recvfrom(fd, in.begin(), in.capacity, MSG_DONTWAIT | MSG_TRUNC,
                         reinterpret_cast<sockaddr*>(&src.sockaddr_), &src_len);

    if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return Opt<Data>{};
    }

    return Opt{Data{static_cast<u64>(ret), rpp::move(src)}};
}

[[nodiscard]] u64 Udp::send(Address address, const Packet& out, u64 length) noexcept {

    i64 ret = ::sendto(fd, out.data(), length, 0,
                       reinterpret_cast<const sockaddr*>(&address.sockaddr_), sizeof(sockaddr_in));
    if(ret == -1) {
        die("Failed send packet: %", Log::sys_error());
    }
    return ret;
}

} // namespace rpp::Net
