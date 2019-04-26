#pragma once

#define NTDDI_VERSION NTDDI_VISTASP1
#define _WIN32_WINNT _WIN32_WINNT_VISTA

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

#define NOMINMAX

// VC++ Runtime Headers
#ifdef _DEBUG
//# define _CRTDBG_MAP_ALLOC
# include <crtdbg.h>
#endif
#include <malloc.h>
//#include <memory.h>
#include <wchar.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <eh.h>
#include <intrin.h>

// STL Headers
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

// Boost Headers
#include <boost/utility.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/random.hpp>
#include <boost/tokenizer.hpp>
#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include "neko_types.h"