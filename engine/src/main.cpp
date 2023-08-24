#include "pch.h"
#include "neko_platform.h"
#include "console.h"
#include "consolewindow.h"
#include "mesh_primitives.h"
#include "engine.h"
#include "locator.h"
#include "memory.h"
#include "filesystem.h"
#include "neko_platform.h"
#include "neko_filepath.h"

using namespace neko;

const string c_consoleThreadName = "nekoConsole";
const string c_consoleTitle      = "nekoengine//console";
const string c_errorTitle        = "nekoengine//exception";
const vec2i  c_consolePos        = { 220, 220 };
const vec2i  c_consoleDims       = { 640, 320 };

#ifdef NEKO_PLATFORM_WINDOWS

// Tell both NVIDIA and AMD to use the most powerful GPU when running on systems with multiple choice (laptops).
extern "C" {
  __declspec( dllexport ) DWORD NvOptimusEnablement = 1;
  __declspec( dllexport ) int AmdPowerXpressRequestHighPerformance = 1;
}

utf8String platform::g_moduleName;

auto g_exceptionReporter = []( string_view description )
{
  if ( Locator::hasConsole() )
  {
    Locator::console().errorPrintf( description.data() );
  }
  platform::showFormattedError( description, c_errorTitle );
};

int consoleThreadSEHFilter( unsigned int code, EXCEPTION_POINTERS* expt )
{
  auto thread = GetCurrentThread();
  platform::SinkedStackWalker sw;
  sw.ShowCallstack( thread, expt->ContextRecord );
  stringstream stream;
  stream << "An exception occurred in the Console thread's message handler.\n";
  stream << "It will be ignored, but this may cause unstability.";
  g_exceptionReporter( stream.str() );
  return EXCEPTION_EXECUTE_HANDLER;
}

void consoleWindowLoopWrap( platform::ConsoleWindow* window, platform::Event& wantStop )
{
  __try
  {
    window->messageLoop( wantStop );
  }
  __except ( consoleThreadSEHFilter( GetExceptionCode(), GetExceptionInformation() ) )
  {
    return;
  }
}

static LONG __stdcall unhandledExceptionHandler( EXCEPTION_POINTERS* expt )
{
  auto thread = GetCurrentThread();
  platform::SinkedStackWalker sw;
  sw.ShowCallstack( thread, expt->ContextRecord );
  stringstream stream;
  stream << "An unhandled exception occurred in " << platform::g_moduleName << ".\n";
  stream << platform::windowsExceptionMessage( thread, expt ) << "\nCall stack:\n" << sw.output();
  g_exceptionReporter( stream.str() );
  FatalAppExitA( -1, stream.str().c_str() );
  return EXCEPTION_CONTINUE_SEARCH;
}

#else

void consoleWindowLoopWrap( platform::ConsoleWindow* window, platform::Event& wantStop )
{
  window->messageLoop( wantStop );
}

#endif

inline int runMain( const std::vector<std::wstring>& arguments )
{
  bool failure = false;

  Environment env;
  env.documentsPath_ = move( platform::getGameDocumentsPath() );
  platform::ensureDirectory( env.documentsPath_ );

  platform::initialize();
  platform::prepareProcess();

  FileSystemPtr globalFileSystem = make_shared<FileSystem>();
  Locator::provideFileSystem( globalFileSystem );

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
    while ( true )
    {
      if ( wantStop.check() )
        break;
      consoleWindowLoopWrap( window.get(), wantStop );
    }
    return true;
  }, console.get() );

  consoleWindowThread.start();

  platform::performanceInitializeGameThread();

  console->printf( srcEngine, R"(Documents directory is %s)", platform::wideToUtf8( env.documentsPath_ ).c_str() );

  EnginePtr engine = make_shared<Engine>( console, env );

  size_t i = 0;
  while ( i < arguments.size() )
  {
    if ( !arguments[i].empty() && arguments[i].c_str()[0] == L'+' )
    {
      i++;
      if ( i < arguments.size() )
      {
        auto varname = platform::wideToUtf8( arguments[i - 1].substr( 1 ) );
        auto var = console->getVariable( varname );
        if ( !var )
          console->printf( srcEngine, R"(Command line "%S": Variable %s not found)", arguments[i - 1].c_str(), varname.c_str() );
        else
          var->set( platform::wideToUtf8( arguments[i] ) );
      }
    }
    else if ( arguments[i] == L"-exec" )
    {
      i++;
      if ( i < arguments.size() )
        console->executeFile( platform::wideToUtf8( arguments[i] ) );
    }
    else
      console->printf( srcEngine, R"(Unknown command line "%S")", arguments[i].c_str() );
    i++;
  }

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
      Locator::console().print( srcEngine, "Shutting down gracefully..." );
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

int APIENTRY wWinMain( HINSTANCE instance, HINSTANCE previous, LPWSTR cmdline, int show )
{
  UNREFERENCED_PARAMETER( previous );
  UNREFERENCED_PARAMETER( cmdline );
  UNREFERENCED_PARAMETER( show );

  // Enable leak checking in debug builds
#ifdef _DEBUG
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

  // CRT memory allocation breakpoints can be set here
  // _CrtSetBreakAlloc( x );

#ifndef _DEBUG
  // Show CRT errors in a message box
  _set_error_mode( _OUT_TO_MSGBOX );

  SetUnhandledExceptionFilter( unhandledExceptionHandler );
#endif
  
  {
    // Get the executable name and save to a global.
    // It's used in error reporting
    wchar_t modname[MAX_PATH];
    GetModuleFileNameW( nullptr, modname, MAX_PATH );
    neko::filepath tmp( modname );
    platform::g_moduleName = tmp.filename().utf8();
  }

  platform::g_instance = instance;

  // Initialize & provide the (ideally pooled) memory service.
  neko::MemoryPtr memoryService = make_shared<Memory>();
  neko::Locator::provideMemory( memoryService );

  int retval = EXIT_SUCCESS;

  NEKO_RELEASE_EXCEPTIONS_TRY
  {
    int argCount;
    auto arguments = CommandLineToArgvW( cmdline, &argCount );
    if ( !arguments )
      return EXIT_FAILURE;

    std::vector<std::wstring> args;
    for ( int i = 0; i < argCount; i++ )
      args.emplace_back( arguments[i] );

    LocalFree( arguments );

    retval = runMain( args );
  }
  NEKO_RELEASE_EXCEPTION_CATCHERS

  return retval;
}

#else
# error Unsupported platform!
#endif