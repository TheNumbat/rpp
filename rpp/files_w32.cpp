
#include "files.h"

#include <windows.h>

namespace rpp::Files {

static wchar_t* utf8_to_ucs2(String_View utf8) {

    constexpr u64 buf_size = 256;
    static thread_local wchar_t wide_buf[buf_size];

    if(!MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(utf8.data()),
                            static_cast<u32>(utf8.length()), wide_buf, buf_size)) {

        warn("Failed to convert utf8 to ucs2: %", Log::sys_error());
        return null;
    }

    return wide_buf;
}

Opt<File_Time> last_write_time(String_View path) {

    WIN32_FILE_ATTRIBUTE_DATA attrib = {};

    wchar_t* ucs2_path = utf8_to_ucs2(path);
    if(!ucs2_path) {
        warn("Failed to convert file path %!", path);
        return {};
    }

    if(GetFileAttributesExW(ucs2_path, GetFileExInfoStandard, (LPVOID)&attrib) == 0) {
        warn("Failed to get file attributes %: %", path, Log::sys_error());
        return {};
    }

    return Opt{(static_cast<u64>(attrib.ftLastWriteTime.dwHighDateTime) << 32) |
               static_cast<u64>(attrib.ftLastWriteTime.dwLowDateTime)};
}

bool before(const File_Time& first, const File_Time& second) {
    FILETIME f, s;
    f.dwLowDateTime = static_cast<u32>(first);
    f.dwHighDateTime = static_cast<u32>(first >> 32);
    s.dwLowDateTime = static_cast<u32>(second);
    s.dwHighDateTime = static_cast<u32>(second >> 32);
    return CompareFileTime(&f, &s) == -1;
}

Opt<Vec<u8, Alloc>> read(String_View path) {

    wchar_t* wide_path = utf8_to_ucs2(path);
    if(!wide_path) {
        warn("Failed to convert file path %!", path);
        return {};
    }

    HANDLE handle =
        CreateFileW(wide_path, GENERIC_READ, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if(handle == INVALID_HANDLE_VALUE) {
        warn("Failed to create file %: %", path, Log::sys_error());
        return {};
    }

    LARGE_INTEGER full_size;
    if(GetFileSizeEx(handle, &full_size) == FALSE) {
        warn("Failed to size file %: %", path, Log::sys_error());
        return {};
    }

    u64 size = static_cast<u64>(full_size.QuadPart);
    assert(size <= UINT32_MAX);

    Vec<u8, Alloc> data(size);
    data.resize(size);

    if(ReadFile(handle, data.data(), static_cast<u32>(size), null, null) == FALSE) {
        warn("Failed to read file %: %", path, Log::sys_error());
        return {};
    }
    CloseHandle(handle);

    return Opt{std::move(data)};
}

bool write(String_View path, Slice<u8> data) {

    wchar_t* wide_path = utf8_to_ucs2(path);
    if(!wide_path) {
        warn("Failed to convert file path %!", path);
        return false;
    }

    HANDLE handle =
        CreateFileW(wide_path, GENERIC_WRITE, 0, null, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, null);
    if(handle == INVALID_HANDLE_VALUE) {
        warn("Failed to create file %: %", path, Log::sys_error());
        return false;
    }

    assert(data.length() <= UINT32_MAX);

    if(WriteFile(handle, data.data(), static_cast<u32>(data.length()), null, null) == FALSE) {
        warn("Failed to write file %: %", path, Log::sys_error());
        return false;
    }
    CloseHandle(handle);

    return true;
}

} // namespace rpp::Files
