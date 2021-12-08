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
    static MeshGeneratorPtr meshGeneratorService_; //!< Currently provided mesh generator service.
    static FileSystemPtr fileSystemService_;
  public:
    static const bool hasMemory() noexcept { return ( memoryService_ ? true : false ); }
    static Memory& memory() { return *memoryService_; }
    static void provideMemory( MemoryPtr memory )
    {
      memoryService_ = move( memory );
    }
    static const bool hasConsole() noexcept { return ( consoleService_ ? true : false ); }
    static Console& console() { return *consoleService_; }
    static void provideConsole( ConsolePtr console )
    {
      consoleService_ = move( console );
    }
    static const bool hasMessaging() noexcept { return ( messagingService_ ? true : false ); }
    static Messaging& messaging() { return *messagingService_; }
    static void provideMessaging ( MessagingPtr messaging )
    {
      messagingService_ = move( messaging );
    }
    static const bool hasDirector() noexcept { return ( directorService_ ? true : false ); }
    static Director& director() { return *directorService_; }
    static void provideDirector( DirectorPtr messaging )
    {
      directorService_ = move( messaging );
    }
    static const bool hasMeshGenerator() noexcept { return ( meshGeneratorService_ ? true : false ); }
    static MeshGenerator& meshGenerator() { return *meshGeneratorService_; }
    static void provideMeshGenerator( MeshGeneratorPtr generator )
    {
      meshGeneratorService_ = move( generator );
    }
    static FileSystem& fileSystem() { return *fileSystemService_; }
    static void provideFileSystem( FileSystemPtr fs )
    {
      fileSystemService_ = move( fs );
    }
  };

}