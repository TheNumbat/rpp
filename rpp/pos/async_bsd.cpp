
#include "../async.h"

#include <sys/event.h>
#include <sys/types.h>
#include <unistd.h>

namespace rpp::Async {

Event::Event() noexcept {
    int pipes[2];
    if(pipe(pipes) == -1) {
        die("Failed to create pipe: %", Log::sys_error());
    }
    fd = pipes[0];
    signal_fd = pipes[1];
    mask = EVFILT_READ;
}

Event::Event(Event&& other) noexcept {
    fd = other.fd;
    other.fd = -1;
    signal_fd = other.signal_fd;
    other.signal_fd = -1;
    mask = other.mask;
    other.mask = 0;
}

Event& Event::operator=(Event&& other) noexcept {
    this->~Event();
    fd = other.fd;
    other.fd = -1;
    signal_fd = other.signal_fd;
    other.signal_fd = -1;
    mask = other.mask;
    other.mask = 0;
    return *this;
}

Event::~Event() noexcept {
    if(fd != -1) close(fd);
    if(signal_fd != -1) close(signal_fd);
    fd = -1;
    signal_fd = -1;
    mask = 0;
}

[[nodiscard]] Event Event::of_sys(i32 fd, i16 mask) noexcept {
    return Event{fd, mask};
}

void Event::signal() const noexcept {
    u8 data = 1;
    assert(write(signal_fd, &data, sizeof(data)) == sizeof(data));
}

void Event::reset() const noexcept {
    u8 data = 0;
    assert(read(fd, &data, sizeof(data)) == sizeof(data));
    assert(data == 1);
}

[[nodiscard]] bool Event::try_wait() const noexcept {
    return false;
}

[[nodiscard]] u64 Event::wait_any(Slice<Event> events) noexcept {

    int kq = kqueue();
    if(kq == -1) {
        die("Failed to create kqueue: %", Log::sys_error());
    }

    struct kevent wait;
    for(auto& e : events) {
        EV_SET(&wait, e.fd, e.mask, EV_ADD | EV_ENABLE, 0, 0, null);
    }

    struct kevent signaled;
    int count = kevent(kq, &wait, 1, &signaled, 1, null);
    if((count < 0) || (signaled.flags == EV_ERROR)) {
        die("Failed to wait on kevents: %", Log::sys_error());
    }
    if(count == 0) {
        return wait_any(events);
    }

    for(u64 i = 0; i < events.length(); ++i) {
        if(events[i].fd == static_cast<i32>(signaled.ident)) {
            return i;
        }
    }
    RPP_UNREACHABLE;
}

} // namespace rpp::Async
