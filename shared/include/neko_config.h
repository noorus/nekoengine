#pragma once

#include "neko_compilerdef.h"

// Set this when compiling on Windows.
// No other platforms are supported at the moment, but unsetting this will show you
// the places where implementations for other platforms would need to be added.
#ifdef _MSC_VER
# define NEKO_PLATFORM_WINDOWS
#endif

// Set this to print extra verbose info/warnings during compilation.
#undef NEKO_VERBOSE_COMPILE

// Set this to catch any new cases of failing backwards compatibility down to Windows Vista that need to be addressed.
#undef NEKO_TEST_WINVISTACOMPAT

// Set this to use doubles for all math primitives. Otherwise using floats.
#undef NEKO_MATH_DOUBLE