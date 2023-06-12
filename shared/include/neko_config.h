#pragma once

#include "neko_compilerdef.h"

// Set this when compiling on Windows.
// No other platforms are supported at the moment, but unsetting this will show you
// the places where implementations for other platforms would need to be added.
#ifdef _MSC_VER
# define NEKO_PLATFORM_WINDOWS
#endif

// Set this to print extra verbose info/warnings during compilation.
#define NEKO_VERBOSE_COMPILE

// Set this to use doubles for all math primitives. Otherwise using floats.
// Will most likely break things.
#undef NEKO_MATH_DOUBLE

// -- Feature switches --
// The game/engine isn't meant to work properly without these, but it is
// convenient to drop some dependencies when developing stuff on the road.

// Define to disable scripting & drop the V8 libraries requirement.
#define NEKO_NO_SCRIPTING

// Define to disable audio & drop FMOD SDK requirement.
#define NEKO_NO_AUDIO

// Define to disable the GUI & drop MyGUI SDK requirement.
#define NEKO_NO_GUI

#define NEKO_NO_RAINET