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

// Set this to use doubles for all math primitives. Otherwise using floats.
#undef NEKO_MATH_DOUBLE

// Set this to disable V8 entirely.
#undef NEKO_NO_SCRIPTING