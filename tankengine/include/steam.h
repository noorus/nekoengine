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

  struct SteamUser
  {
    CSteamID id_;
    utf8String name_;
    utf8String displayName_;
  };

  class SteamState {
  public:
    SteamUser localUser_;
    union Updated
    {
      struct Bits
      {
        uint8_t self : 1;
        uint8_t friends : 1;
        uint8_t images : 1;
      } bits;
      uint64_t field;
    } updated_;
    map<SteamSnowflake, SteamUser> friends_;
    map<SteamSnowflake, Image> userImages_;
  };

  class Steam {
  private:
    static TankHost* host_;
    uint32_t appID_;
    SteamState state_;
    GameInstallationState installation_;
    utf8String commandLine_;
    STEAM_CALLBACK( Steam, OnGameOverlayActivated, GameOverlayActivated_t );
    STEAM_CALLBACK( Steam, OnIPCFailure, IPCFailure_t ); // respond by killing steam and reconnecting
    STEAM_CALLBACK( Steam, OnLicensesUpdated, LicensesUpdated_t );
    STEAM_CALLBACK( Steam, OnSteamServersConnected, SteamServersConnected_t );
    STEAM_CALLBACK( Steam, OnSteamServersDisconnected, SteamServersDisconnected_t );
    STEAM_CALLBACK( Steam, OnNewUrlLaunchParameters, NewUrlLaunchParameters_t );
    static void __cdecl staticSteamAPIWarningHook( int severity, const char* message );
  protected:
    void fetchImage( SteamUser& user );
  public:
    Steam( uint32_t applicationID, TankHost* host );
    static void baseInitialize();
    static void baseShutdown();
    inline const GameInstallationState& installation() throw() { return installation_; }
    inline const utf8String& commandline() throw() { return commandLine_; }
    void initialize();
    void update();
    void shutdown();
    ~Steam();
  };

}