
#include "../asyncio.h"

#include "w32_util.h"
#include <windows.h>

namespace rpp::AsyncIO {

constexpr u64 SECTOR_SIZE = 4096;

Async::Task<Opt<Vec<u8, Alloc>>> read(Thread::Pool<>& pool, String_View path) {

    auto [ucs2_path, ucs2_path_len] = utf8_to_ucs2(path);
    if(ucs2_path_len == 0) {
        warn("Failed to convert file path %!", path);
        co_return {};
    }

    HANDLE handle =
        CreateFileW(ucs2_path, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, null);
    if(handle == INVALID_HANDLE_VALUE) {
        warn("Failed to create file %: %", path, Log::sys_error());
        co_return {};
    }

    LARGE_INTEGER full_size;
    if(GetFileSizeEx(handle, &full_size) == FALSE) {
        warn("Failed to size file %: %", path, Log::sys_error());
        co_return {};
    }

    u64 size = static_cast<u64>(full_size.QuadPart);
    u64 aligned_size = Math::align_pow2(size, SECTOR_SIZE);

    assert(aligned_size <= UINT32_MAX);

    Vec<u8, Alloc> data(aligned_size);
    data.resize(aligned_size);

    HANDLE event = CreateEventEx(null, null, 0, EVENT_ALL_ACCESS);
    if(!event) {
        warn("Failed to create event: %", Log::sys_error());
        co_return {};
    }

    OVERLAPPED overlapped = {};
    overlapped.hEvent = event;

    BOOL ret = ReadFile(handle, data.data(), static_cast<u32>(aligned_size), null, &overlapped);
    if(ret == TRUE || GetLastError() != ERROR_IO_PENDING) {
        warn("Failed to initiate async read of file %: %", path, Log::sys_error());
        CloseHandle(event);
        CloseHandle(handle);
        co_return {};
    }

    co_await pool.event(Async::Event::of_sys(event));

    CloseHandle(handle);
    data.resize(size);
    co_return Opt{std::move(data)};
}

Async::Task<bool> write(Thread::Pool<>& pool, String_View path, Slice<u8> data) {

    auto [ucs2_path, ucs2_path_len] = utf8_to_ucs2(path);
    if(ucs2_path_len == 0) {
        warn("Failed to convert file path %!", path);
        co_return false;
    }

    HANDLE handle =
        CreateFileW(ucs2_path, GENERIC_WRITE, 0, null, CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, null);
    if(handle == INVALID_HANDLE_VALUE) {
        warn("Failed to create file %: %", path, Log::sys_error());
        co_return false;
    }

    HANDLE event = CreateEventEx(null, null, 0, EVENT_ALL_ACCESS);
    if(!event) {
        warn("Failed to create event: %", Log::sys_error());
        co_return false;
    }

    OVERLAPPED overlapped = {};
    overlapped.hEvent = event;

    u64 size = data.length();
    u64 aligned_size = Math::align_pow2(size, SECTOR_SIZE);
    assert(aligned_size <= UINT32_MAX);

    Vec<u8, Alloc> to_write(aligned_size);
    to_write.resize(aligned_size);

    Std::memcpy(to_write.data(), data.data(), size);

    BOOL ret =
        WriteFile(handle, to_write.data(), static_cast<u32>(aligned_size), null, &overlapped);
    if(ret == TRUE || GetLastError() != ERROR_IO_PENDING) {
        warn("Failed to initiate async write of file %: %", path, Log::sys_error());
        CloseHandle(event);
        CloseHandle(handle);
        co_return false;
    }

    co_await pool.event(Async::Event::of_sys(event));

    if(SetFilePointer(handle, static_cast<u32>(size), null, FILE_BEGIN) ==
       INVALID_SET_FILE_POINTER) {
        warn("Failed to set file pointer for file %: %", path, Log::sys_error());
        CloseHandle(handle);
        co_return false;
    }

    if(SetEndOfFile(handle) == FALSE) {
        warn("Failed to set end of file for file %: %", path, Log::sys_error());
        CloseHandle(handle);
        co_return false;
    }

    CloseHandle(handle);
    co_return true;
}

Async::Task<void> wait(Thread::Pool<>& pool, u64 ms) {

    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if(timer == INVALID_HANDLE_VALUE) {
        warn("Failed to create waitable timer: %", Log::sys_error());
        co_return;
    }

    LARGE_INTEGER liDueTime;
    liDueTime.QuadPart = -static_cast<LONGLONG>(ms * 10000);

    if(SetWaitableTimer(timer, &liDueTime, 0, NULL, NULL, 0) == FALSE) {
        warn("Failed to set waitable timer: %", Log::sys_error());
        CloseHandle(timer);
        co_return;
    }

    co_await pool.event(Async::Event::of_sys(timer));

    CloseHandle(timer);
    co_return;
}

} // namespace rpp::AsyncIO
