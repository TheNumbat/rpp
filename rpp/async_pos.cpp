
#include "async.h"

namespace rpp::Async {

Event::~Event() {
}

Event::Event() {
}

Event Event::of_sys(i32 fd) {
    return Event{fd};
}

void Event::signal() const {
}

bool Event::ready() const {
    return false;
}

void Event::wait() const {
}

void Event::wait_all(Slice<Event> events) {
}

u64 Event::wait_any(Slice<Event> events) {
    return 0;
}

} // namespace rpp::Async
