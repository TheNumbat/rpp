
#include "../async.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace rpp::Async {

Event::Event() noexcept {
    int event = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if(event == -1) {
        die("Failed to create event: %", Log::sys_error());
    }
    fd = event;
    mask = EPOLLIN;
}

Event::Event(Event&& other) noexcept {
    fd = other.fd;
    other.fd = -1;
    mask = other.mask;
    other.mask = 0;
}

Event& Event::operator=(Event&& other) noexcept {
    this->~Event();
    fd = other.fd;
    other.fd = -1;
    mask = other.mask;
    other.mask = 0;
    return *this;
}

Event::~Event() noexcept {
    if(fd != -1) {
        int ret = close(fd);
        assert(ret == 0);
    }
    fd = -1;
    mask = 0;
}

[[nodiscard]] Event Event::of_sys(i32 fd, i32 mask) noexcept {
    return Event{fd, mask};
}

void Event::signal() const noexcept {
    u64 value = 1;
    int ret = write(fd, &value, sizeof(value));
    if(ret == -1) {
        die("Failed to signal event: %", Log::sys_error());
    }
}

void Event::reset() const noexcept {
    u64 value = 0;
    int ret = read(fd, &value, sizeof(value));
    if(ret == -1) {
        die("Failed to reset event: %", Log::sys_error());
    }
}

[[nodiscard]] bool Event::try_wait() const noexcept {
    return false;
}

[[nodiscard]] u64 Event::wait_any(Slice<Event> events) noexcept {

    int epfd = epoll_create1(EPOLL_CLOEXEC);
    if(epfd == -1) {
        die("Failed to create epoll: %", Log::sys_error());
    }

    for(auto& event : events) {
        epoll_event ev;
        ev.events = event.mask;
        ev.data.fd = event.fd;
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, event.fd, &ev);
        if(ret == -1) {
            die("Failed to add event to epoll: %", Log::sys_error());
        }
    }

    epoll_event ev;
    int ret = epoll_wait(epfd, &ev, 1, -1);

    if(ret == -1) {
        die("Failed to wait on events: %", Log::sys_error());
    }

    ret = close(epfd);
    assert(ret == 0);

    for(u64 i = 0; i < events.length(); ++i) {
        if(events[i].fd == ev.data.fd) {
            return i;
        }
    }
    RPP_UNREACHABLE;
}

} // namespace rpp::Async
