#pragma once
#include "neko_types.h"
#include "forwards.h"

namespace neko {

  //! \class Locator
  //! \brief Central game services locator.
  class Locator {
  private:
    static MemoryPtr memoryService_; //!< Currently provided memory service.
  public:
    inline const bool hasMemory() const throw() { return ( memoryService_ ? true : false ); }
    inline Memory& getMemory() { return *memoryService_; }
    static void provideMemory( MemoryPtr memory )
    {
      memoryService_ = move( memory );
    }
  };

}