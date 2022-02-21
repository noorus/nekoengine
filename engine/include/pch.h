#pragma once

#include "neko_config.h"

#ifdef NEKO_VERBOSE_COMPILE
# define GLM_FORCE_MESSAGES
#endif

// Platform specifics
#ifdef NEKO_PLATFORM_WINDOWS

#if !defined(_DEBUG) && !defined(NEKO_VERBOSE_COMPILE)
# define _CRT_SECURE_NO_WARNINGS
# define _SCL_SECURE_NO_WARNINGS
#endif

#define NTDDI_VERSION NTDDI_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10

#include <sdkddkver.h>

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <richedit.h>
#include <ctime>
//#include <psapi.h>
//#include <dbghelp.h>
#include <avrt.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// VC++ Runtime Headers
#ifdef _DEBUG
//# define _CRTDBG_MAP_ALLOC
# include <crtdbg.h>
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
#include <intrin.h>
#include <cassert>

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
#include <chrono>
#include <span>

// OpenGL
#include <glbinding/gl45core/gl.h>
#include <glbinding/glbinding.h>
#ifndef RELEASE
# include <glbinding-aux/Meta.h>
#endif

// SFML
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

// FMOD
#ifndef NEKO_NO_AUDIO
# include <fmod.hpp>
# include <fmod_errors.h>
# include <fmod_studio.hpp>
# include <fsbank.h>
# include <fsbank_errors.h>
#endif

#pragma warning( push )
#pragma warning( disable: 4251 )

// V8
#ifndef NEKO_NO_SCRIPTING
# include <libplatform/libplatform.h>
# include <v8.h>
#endif

// ICU
#ifndef NEKO_NO_ICU
# include <unicode/utypes.h>
# include <unicode/uchar.h>
# include <unicode/locid.h>
# include <unicode/ustring.h>
# include <unicode/ucnv.h>
# include <unicode/unistr.h>
# include <unicode/utf8.h>
# include <unicode/utf16.h>
#endif // !NEKO_NO_ICU

#pragma warning( pop )

// Newtype
#include <newtype.h>

// Nil
#include <nil.h>

// MyGUI
#ifndef NEKO_NO_GUI
# pragma warning( push )
# pragma warning( disable: 4275 )
# include <MYGUI/MyGUI.h>
# pragma warning( pop )
#endif

// JSON
#include <json.hpp>

// Local types
#include "neko_types.h"
#include "nekomath.h"