#pragma once

#include "tankengine.h"

#pragma warning( push )
#pragma warning( disable : 4819 )
#include "steam/steam_api_common.h"
#include "steam/isteamapps.h"
#include "steam/isteamfriends.h"
#include "steam/isteamuser.h"
#pragma warning( pop )

namespace tank {

  class Steam {
  private:
    static TankHost* host_;
    uint32_t appID_;
    GameInstallationState installation_;
    STEAM_CALLBACK( Steam, OnGameOverlayActivated, GameOverlayActivated_t );
    STEAM_CALLBACK( Steam, OnIPCFailure, IPCFailure_t ); // respond by killing steam and reconnecting
    STEAM_CALLBACK( Steam, OnLicensesUpdated, LicensesUpdated_t );
    STEAM_CALLBACK( Steam, OnSteamServersConnected, SteamServersConnected_t );
    STEAM_CALLBACK( Steam, OnSteamServersDisconnected, SteamServersDisconnected_t );
    static void __cdecl staticSteamAPIWarningHook( int severity, const char* message );
  public:
    Steam( uint32_t applicationID, TankHost* host );
    static void baseInitialize();
    static void baseShutdown();
    inline const GameInstallationState& installation() throw() { return installation_; }
    void initialize();
    void update();
    void shutdown();
    ~Steam();
  };

}