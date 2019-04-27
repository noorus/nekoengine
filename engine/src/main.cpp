#include "stdafx.h"
#include "neko_platform.h"
#include "console.h"
#include "consolewindow.h"
#include "engine.h"

using namespace neko;

const string c_consoleThreadName = "nekoConsole";
const string c_consoleTitle = "nekoengine//console";

inline int runMain()
{
  platform::initialize();
  platform::prepareProcess();

  /*Console console;

  platform::Thread consoleWindowThread( c_consoleThreadName,
    []( platform::Event& running, platform::Event& wantStop, void* argument ) -> bool
  {
    auto cnsl = (Console*)argument;
    platform::ConsoleWindowPtr window = make_shared<platform::ConsoleWindow>( cnsl, c_consoleTitle, 220, 220, 640, 320 );
    running.set();
    window->messageLoop( wantStop );
    return true;
  }, &console );

  consoleWindowThread.start();*/

  EnginePtr engine = make_shared<Engine>();
  engine->initialize( Engine::Options() );
  engine->run();
  engine->shutdown();
  engine.reset();

  /*while ( true )
  {
    if ( !consoleWindowThread.check() )
      break;
    Sleep( 1000 );
  }

  consoleWindowThread.stop();*/

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
  //_CrtSetBreakAlloc( x );

  neko::platform::g_instance = hInstance;

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
    MessageBoxA( 0, e.getFullDescription().c_str(), "Exception", MB_ICONERROR | MB_OK | MB_TASKMODAL );
    return EXIT_FAILURE;
  }
  catch ( ... )
  {
    MessageBoxA( 0, "Unknown exception!", "Exception", MB_ICONERROR | MB_OK | MB_TASKMODAL );
    return EXIT_FAILURE;
  }
#endif

  return retval;
}

#else
# error Unsupported platform!
#endif