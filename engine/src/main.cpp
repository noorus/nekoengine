#include "stdafx.h"
#include "neko_platform.h"
#include "console.h"
#include "consolewindow.h"
#include "engine.h"
#include "locator.h"
#include "memory.h"

using namespace neko;

const string c_consoleThreadName = "nekoConsole";
const string c_consoleTitle      = "nekoengine//console";
const string c_errorTitle        = "nekoengine//exception";
const vec2i  c_consolePos        = { 220, 220 };
const vec2i  c_consoleDims       = { 640, 320 };

// Tell both NVIDIA and AMD to use the most powerful GPU when running on systems with multiple choice (laptops).
extern "C" {
  __declspec( dllexport ) DWORD NvOptimusEnablement = 1;
  __declspec( dllexport ) int AmdPowerXpressRequestHighPerformance = 1;
}

auto g_exceptionReporter = []( string_view description )
{
  if ( Locator::hasConsole() )
  {
    Locator::console().errorPrintf( description.data() );
  }
  platform::errorBox( description, c_errorTitle );
};

inline int runMain()
{
  bool failure = false;

  platform::initialize();
  platform::prepareProcess();

  ConsolePtr console = make_shared<Console>();
  Locator::provideConsole( console );

  MeshGeneratorPtr globalMeshGenerator = make_shared<MeshGenerator>();
  Locator::provideMeshGenerator( globalMeshGenerator );

  platform::performanceInitializeProcess();

  platform::Thread consoleWindowThread( c_consoleThreadName,
    []( platform::Event& running, platform::Event& wantStop, void* argument ) -> bool
  {
    auto console = ( (Console*)argument )->shared_from_this();
    platform::ConsoleWindowPtr window = make_shared<platform::ConsoleWindow>(
      console, c_consoleTitle,
      (int)c_consolePos[0], (int)c_consolePos[1], // x, y
      (int)c_consoleDims[0], (int)c_consoleDims[1] // w, h
    );
    running.set();
    window->messageLoop( wantStop );
    return true;
  }, console.get() );

  consoleWindowThread.start();

  platform::performanceInitializeGameThread();

  EnginePtr engine = make_shared<Engine>( console );

#ifndef _DEBUG
  try
#endif
  {
    engine->initialize( Engine::Options() );
    engine->run();
  }
#ifndef _DEBUG
  catch ( neko::Exception& e )
  {
    g_exceptionReporter( e.getFullDescription() );
    if ( Locator::hasConsole() )
      Locator::console().print( Console::srcEngine, "Shutting down gracefully..." );
    failure = true;
  }
#endif

  if ( consoleWindowThread.check() )
    consoleWindowThread.stop();
  consoleWindowThread.waitFor();

  if ( engine )
  {
    engine->shutdown();
    engine.reset();
  }

  platform::performanceTeardownCurrentThread();

  platform::performanceTeardownProcess();

  Locator::provideConsole( ConsolePtr() );
  console.reset();

  platform::shutdown();

  return ( failure ? EXIT_FAILURE : EXIT_SUCCESS );
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

#ifndef _DEBUG
  try
#endif
  {
    retval = runMain();
  }
#ifndef _DEBUG
  catch ( neko::Exception& e )
  {
    g_exceptionReporter( e.getFullDescription() );
    return EXIT_FAILURE;
  }
  catch ( ... )
  {
    g_exceptionReporter( "Unknown exception!" );
    return EXIT_FAILURE;
  }
#endif

  return retval;
}

#else
# error Unsupported platform!
#endif