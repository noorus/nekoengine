#include "stdafx.h"
#include "tankengine.h"

const uint32_t c_libraryVersion = 1;
static std::unique_ptr<tank::TankEngine> g_instance;

extern "C" {

  TANK_EXPORT tank::TankEngine* TANK_CALL tankInitialize( uint32_t version, tank::TankHost* host )
  {
    if ( version != c_libraryVersion )
      return nullptr;

    g_instance = std::make_unique<tank::TankEngine>( host );
    return g_instance.get();
  }

  TANK_EXPORT void TANK_CALL tankShutdown( tank::TankEngine* engine )
  {
    g_instance.reset();
  }

}

BOOL APIENTRY DllMain( HMODULE module, DWORD reason, LPVOID reserved )
{
  switch ( reason )
  {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      g_instance.reset();
      break;
  }
  return TRUE;
}