#pragma once
#include <map>
#include <string>

// This is uncharted territory;
// Don't do this unless you're ready to abandon all the neat Valve-provided macros for callback handling,
// and ready to manually update all the stuff that was uprooted and placed here from steam_api.h

namespace steam {

#pragma warning( push )
#pragma warning( disable : 4819 )
#pragma warning( disable : 4996 )
#pragma warning( disable : 4828 )
#include <steam/steam_api_common.h>
#include <steam/isteamapps.h>
#include <steam/isteamclient.h>
#include <steam/isteamcontroller.h>
#include <steam/isteamdualsense.h>
#include <steam/isteamfriends.h>
#include <steam/isteaminput.h>
#include <steam/isteaminventory.h>
#include <steam/isteammatchmaking.h>
#include <steam/isteammusic.h>
#include <steam/isteammusicremote.h>
#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/isteamremoteplay.h>
#include <steam/isteamremotestorage.h>
#include <steam/isteamscreenshots.h>
#include <steam/isteamugc.h>
#include <steam/isteamuser.h>
#include <steam/isteamuserstats.h>
#include <steam/isteamutils.h>
#include <steam/isteamvideo.h>
#include <steam/steamclientpublic.h>
#pragma warning( pop )

}

namespace steam {

  enum EServerMode
  {
    eServerModeInvalid = 0, // DO NOT USE
    eServerModeNoAuthentication = 1, // Don't authenticate user logins and don't list on the server list
    eServerModeAuthentication = 2, // Authenticate users, list on the server list, don't run VAC on clients that connect
    eServerModeAuthenticationAndSecure = 3, // Authenticate users, list on the server list and VAC protect clients
  };

  const uint16_t STEAMGAMESERVER_QUERY_PORT_SHARED = 0xffff;
  const uint16_t MASTERSERVERUPDATERPORT_USEGAMESOCKETSHARE = STEAMGAMESERVER_QUERY_PORT_SHARED;

  extern const std::map<EResult, std::string> c_steamEResultDescriptionMap;

}

S_API bool S_CALLTYPE SteamAPI_Init();
S_API void S_CALLTYPE SteamAPI_Shutdown();

S_API void S_CALLTYPE SteamAPI_ReleaseCurrentThreadMemory();
S_API bool S_CALLTYPE SteamAPI_RestartAppIfNecessary( steam::uint32 unOwnAppID );

S_API void S_CALLTYPE SteamAPI_ManualDispatch_Init();
S_API void S_CALLTYPE SteamAPI_ManualDispatch_RunFrame( steam::HSteamPipe hSteamPipe );
S_API bool S_CALLTYPE SteamAPI_ManualDispatch_GetNextCallback(
  steam::HSteamPipe hSteamPipe, steam::CallbackMsg_t* pCallbackMsg );
S_API void S_CALLTYPE SteamAPI_ManualDispatch_FreeLastCallback( steam::HSteamPipe hSteamPipe );
S_API bool S_CALLTYPE SteamAPI_ManualDispatch_GetAPICallResult( steam::HSteamPipe hSteamPipe,
  steam::SteamAPICall_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed );