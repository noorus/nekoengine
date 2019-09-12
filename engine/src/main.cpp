#include "stdafx.h"
#include "neko_platform.h"
#include "console.h"
#include "consolewindow.h"
#include "engine.h"
#include "locator.h"
#include "memory.h"

using namespace neko;

const string c_consoleThreadName = "nekoConsole";
const string c_consoleTitle = "nekoengine//console";

inline int runMain()
{
  platform::initialize();
  platform::prepareProcess();

  ConsolePtr console = make_shared<Console>();

  Locator::provideConsole( console );

  platform::Thread consoleWindowThread( c_consoleThreadName,
    []( platform::Event& running, platform::Event& wantStop, void* argument ) -> bool
  {
    auto console = ( (Console*)argument )->shared_from_this();
    platform::ConsoleWindowPtr window = make_shared<platform::ConsoleWindow>( console, c_consoleTitle, 220, 220, 640, 320 );
    running.set();
    window->messageLoop( wantStop );
    return true;
  }, console.get() );

  consoleWindowThread.start();

  EnginePtr engine = make_shared<Engine>( console );
  engine->initialize( Engine::Options() );
  engine->run();

  if ( consoleWindowThread.check() )
    consoleWindowThread.stop();
  consoleWindowThread.waitFor();

  engine->shutdown();
  engine.reset();

  Locator::provideConsole( ConsolePtr() );
  console.reset();

  platform::shutdown();

  return 0;
}

#ifdef NEKO_PLATFORM_WINDOWS

int APIENTRY wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
  UNREFERENCED_PARAMETER( hPrevInstance );
  UNREFERENCED_PARAMETER( lpCmdLine );
  UNREFERENCED_PARAMETER( nCmdShow );

  // Enable leak checking in debug builds
#if defined( _DEBUG ) || defined( DEBUG )
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

  // CRT memory allocation breakpoints can be set here
  // _CrtSetBreakAlloc( x );

  platform::g_instance = hInstance;

  // Initialize & provide the (ideally pooled) memory service.
  neko::MemoryPtr memoryService = make_shared<Memory>();
  neko::Locator::provideMemory( memoryService );

  int retval = EXIT_SUCCESS;

  auto exceptionReporter = []( string_view description )
  {
    if ( Locator::hasConsole() )
    {
      Locator::console().printf( Console::srcEngine, "Fatal: %s", description.data() );
    }
    platform::errorBox( description, "Exception" );
  };

#ifndef _DEBUG
  try
#endif
  {
    retval = runMain();
  }
#ifndef _DEBUG
  catch ( neko::Exception& e )
  {
    exceptionReporter( e.getFullDescription() );
    return EXIT_FAILURE;
  }
  catch ( ... )
  {
    exceptionReporter( "Unknown exception!" );
    return EXIT_FAILURE;
  }
#endif

  return retval;
}

#else
# error Unsupported platform!
#endif