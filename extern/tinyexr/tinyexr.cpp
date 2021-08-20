#include "stdafx.h"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#define TINYEXR_USE_MINIZ 1
#define TINYEXR_USE_PIZ 1
#define TINYEXR_USE_ZFP 0
#define TINYEXR_USE_THREAD 1
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
