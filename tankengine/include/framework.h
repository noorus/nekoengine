#pragma once

#include "neko_config.h"

// Platform specifics
#ifdef NEKO_PLATFORM_WINDOWS

#if !defined( _DEBUG ) && !defined( NEKO_VERBOSE_COMPILE )
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif
#endif

#define NTDDI_VERSION NTDDI_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10

#include <sdkddkver.h>

#include <windows.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// VC++ Runtime Headers
#ifdef _DEBUG
//# define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#else
# error Unknown platform!
#endif

#include <cstdio>
#include <malloc.h>
#include <memory.h>
#include <cwchar>
#define _USE_MATH_DEFINES
#include <cmath>
#include <eh.h>
#include <cassert>
#include <intrin.h>

#undef min
#undef max

// STL
#include <exception>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <regex>
#include <stack>
#include <cstdint>
#include <algorithm>
#include <random>
#include <filesystem>
#include <queue>
#include <unordered_map>
#include <utility>
#include <variant>
#include <any>
#include <optional>

// GSL
#include <gsl/gsl>

#pragma warning( push )
#pragma warning( disable : 4251 )

#ifndef NEKO_NO_ICU
// ICU
#include <unicode/utypes.h>
#include <unicode/uchar.h>
#include <unicode/locid.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>
#include <unicode/unistr.h>
#include <unicode/utf8.h>
#include <unicode/utf16.h>
#endif // !NEKO_NO_ICU

#pragma warning( pop )