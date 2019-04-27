#pragma once
#include "neko_types.h"

namespace neko {

  class Subsystem;
  using SubsystemPtr = shared_ptr<Subsystem>;

  class Engine;
  using EnginePtr = shared_ptr<Engine>;

  class Gfx;
  using GfxPtr = shared_ptr<Gfx>;

  class Console;
  using ConsolePtr = shared_ptr<Console>;

}