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

#ifdef NEKO_TEST_WINVISTACOMPAT
# define NTDDI_VERSION NTDDI_VISTASP4
# define _WIN32_WINNT _WIN32_WINNT_VISTA
#else
# define NTDDI_VERSION NTDDI_WIN10
# define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif
#include <sdkddkver.h>

#include <windows.h>          // Windows
#include <shellapi.h>         // ShellAPI
#include <shlobj.h>           // ShellAPI OLE Objects
#include <commctrl.h>         // Common Controls          (comctl32.lib)
#include <richedit.h>         // RichEdit
#include <time.h>             // Time
//#include <psapi.h>            // Process Status API       (psapi.lib)
//#include <dbghelp.h>          // Debug Help Library       (dbghelp.lib)
#include <avrt.h>             // AVRT                     (avrt.lib)

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

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <eh.h>
#include <intrin.h>
#include <assert.h>

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

// Boost
#include <boost/utility.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/random.hpp>
#include <boost/tokenizer.hpp>
#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

// GLM
#define GLM_FORCE_SIZE_T_LENGTH
#define GLM_FORCE_RADIANS 1
#define GLM_FORCE_UNRESTRICTED_GENTYPE
#if !defined(NEKO_VERBOSE_COMPILE) && !defined(_DEBUG)
# define GLM_FORCE_SILENT_WARNINGS
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// FMOD
#include <fmod.hpp>
#include <fmod_errors.h>
#include <fmod_studio.hpp>
#include <fsbank.h>
#include <fsbank_errors.h>

// V8
#pragma warning( push )
#pragma warning( disable: 4251 )
#include <libplatform/libplatform.h>
#include <v8.h>
#pragma warning( pop )

// Local types
#include "neko_types.h"
#include "nekomath.h"