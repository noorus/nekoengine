#include "stdafx.h"
#include "neko_platform.h"

inline int runMain()
{
  neko::platform::initialize();
  neko::platform::prepareProcess();

  MessageBox( 0, L"hello", L"", MB_OK );

  neko::platform::shutdown();

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