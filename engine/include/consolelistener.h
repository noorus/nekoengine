#pragma once
#include "neko_types.h"

namespace neko {

  class Console;

  class ConsoleListener {
  public:
    virtual void onConsolePrint( Console* console, vec3 color, const string& str ) = 0;
  };

  using ConsoleListenerPtr = shared_ptr<ConsoleListener>;

}