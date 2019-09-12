#pragma once
#include "neko_types.h"
#include "forwards.h"

namespace neko {

  //! \class Locator
  //! \brief Central game services locator.
  class Locator {
  private:
    static MemoryPtr memoryService_; //!< Currently provided memory service.
    static ConsolePtr consoleService_; //!< Currently provided console service.
  public:
    static const bool hasMemory() throw() { return ( memoryService_ ? true : false ); }
    static Memory& memory() { return *memoryService_; }
    static void provideMemory( MemoryPtr memory )
    {
      memoryService_ = move( memory );
    }
    static const bool hasConsole() throw() { return ( consoleService_ ? true : false ); }
    static Console& console() { return *consoleService_; }
    static void provideConsole( ConsolePtr console )
    {
      consoleService_ = move( console );
    }
  };

}