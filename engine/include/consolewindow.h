#pragma once

#ifdef NEKO_PLATFORM_WINDOWS
# include "consolewindow_windows.h"
#else
# error Unknown platform!
#endif

namespace neko {

  namespace platform {

    using ConsoleWindowPtr = shared_ptr<ConsoleWindow>;

  }

}