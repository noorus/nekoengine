#include "pch.h"
#include "rainet.h"

const uint32_t c_libraryVersion = rainet::c_headerVersion;
static std::unique_ptr<rainet::System> g_instance;

extern "C" {

  RAINET_EXPORT rainet::System* RAINET_CALL rainetInitialize( uint32_t version, rainet::Host* host )
  {
    if ( version != c_libraryVersion )
      return nullptr;

    g_instance = std::make_unique<rainet::System>( host );
    return g_instance.get();
  }

  RAINET_EXPORT void RAINET_CALL rainetShutdown( rainet::System* engine )
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