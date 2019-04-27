#pragma once

// Set this when compiling on Windows.
// No other platforms are supported at the moment, but unsetting this will show you
// the places where implementations for other platforms would need to be added.
#define NEKO_PLATFORM_WINDOWS

// Set this to print extra verbose info/warnings during compilation.
#define NEKO_VERBOSE_COMPILE

// Set this to catch any new cases of failing backwards compatibility down to Windows Vista that need to be addressed.
#undef NEKO_TEST_WINVISTACOMPAT