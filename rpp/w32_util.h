
#pragma once

#include "base.h"

namespace rpp {

Pair<wchar_t*, int> utf8_to_ucs2(String_View utf8);
String_View ucs2_to_utf8(const wchar_t* ucs2, int ucs2_len);
String_View basic_win32_error(u32 err);

} // namespace rpp
