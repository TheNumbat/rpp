
#include "../async.h"

#include <windows.h>

namespace rpp::Async {

static_assert(sizeof(HANDLE) == sizeof(void*));

Event::Event() {
    HANDLE event = CreateEventEx(null, null, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
    if(!event) {
        die("Failed to create event: %", Log::sys_error());
    }
    event_ = reinterpret_cast<void*>(event);
}

Event::Event(Event&& other) {
    event_ = other.event_;
    other.event_ = null;
}

Event& Event::operator=(Event&& other) {
    this->~Event();
    event_ = other.event_;
    other.event_ = null;
    return *this;
}

Event::~Event() {
    HANDLE event = reinterpret_cast<HANDLE>(event_);
    if(event) {
        BOOL ret = CloseHandle(event);
        assert(ret);
    }
    event_ = null;
}

Event Event::of_sys(void* handle) {
    assert(handle);
    return Event{handle};
}

void Event::reset() const {
    HANDLE event = reinterpret_cast<HANDLE>(event_);
    BOOL ret = ResetEvent(event);
    if(!ret) {
        die("Failed to reset event: %", Log::sys_error());
    }
}

void Event::signal() const {
    HANDLE event = reinterpret_cast<HANDLE>(event_);
    BOOL ret = SetEvent(event);
    if(!ret) {
        die("Failed to signal event: %", Log::sys_error());
    }
}

bool Event::try_wait() const {
    HANDLE event = reinterpret_cast<HANDLE>(event_);
    DWORD ret = WaitForSingleObjectEx(event, 0, false);
    if(ret == WAIT_OBJECT_0) {
        return true;
    } else if(ret == WAIT_TIMEOUT) {
        return false;
    } else {
        die("Failed to check event ready: % (%)", static_cast<u32>(ret), Log::sys_error());
    }
}

u64 Event::wait_any(Slice<Event> events) {
    assert(!events.empty());
    const HANDLE* handles = reinterpret_cast<const HANDLE*>(events.data());
    DWORD ret = WaitForMultipleObjectsEx(static_cast<DWORD>(events.length()), handles, false,
                                         INFINITE, false);
    if(ret < WAIT_OBJECT_0 || ret >= WAIT_OBJECT_0 + events.length()) {
        die("Failed to wait on events: % (%)", static_cast<u32>(ret), Log::sys_error());
    }
    return ret - WAIT_OBJECT_0;
}

} // namespace rpp::Async
