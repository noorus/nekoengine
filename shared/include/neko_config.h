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
// Will most likely break things.
#undef NEKO_MATH_DOUBLE

// -- Platform/Store integrations --
#undef NEKO_USE_DISCORD
#undef NEKO_USE_STEAM

// -- Feature switches --
// The game/engine isn't meant to work properly without these, but it is
// convenient to drop some dependencies when developing stuff on the road.

// Define to disable scripting & drop the V8 libraries requirement.
#undef NEKO_NO_SCRIPTING

// Define to disable audio & drop FMOD SDK requirement.
#define NEKO_NO_AUDIO

// Define to disable the GUI & drop MyGUI SDK requirement.
#undef NEKO_NO_GUI

// Define to disable the OZZ animation system stuff. Will probably break momentarily.
#define NEKO_NO_ANIMATION

#define NEKO_NO_FBX

// Intel graphics drivers shit the bed every time when freeing stuff on exit.
// Set this to force an ExitProcess() instead when debugging on an Intel chip in order to not waste time.
// EDIT: This is now a project-level preprocessor directive, don't set or unset it here.
// Use the ShittyDebug build profile instead.
// #define INTEL_SUCKS