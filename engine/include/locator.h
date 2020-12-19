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
    static MessagingPtr messagingService_; //!< Currently provided messaging service.
    static DirectorPtr directorService_; //!< Currently provided director service.
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
    static const bool hasMessaging() throw( ) { return ( messagingService_ ? true : false ); }
    static Messaging& messaging() { return *messagingService_; }
    static void provideMessaging ( MessagingPtr messaging )
    {
      messagingService_ = move( messaging );
    }
    static const bool hasDirector() throw() { return ( directorService_ ? true : false ); }
    static Director& director() { return *directorService_; }
    static void provideDirector( DirectorPtr messaging )
    {
      directorService_ = move( messaging );
    }
  };

}