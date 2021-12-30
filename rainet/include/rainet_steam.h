#pragma once

#include "rainet.h"

#pragma warning( push )
#pragma warning( disable : 4819 )
#include "steam/steam_api_common.h"
#include "steam/isteamapps.h"
#include "steam/isteamfriends.h"
#include "steam/isteamuser.h"
#include "steam/isteamuserstats.h"
#include "steam/isteaminventory.h"
#include "steam/isteammatchmaking.h"
#pragma warning( pop )

namespace rainet {

  struct SteamUser
  {
    CSteamID id_;
    utf8String name_;
    utf8String displayName_;
  };

  struct SteamStats
  {
    SteamSnowflake id_;
    map<utf8String, SteamStat> map_;
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
        uint8_t stats : 1;
        uint8_t playerCount : 1;
      } bits;
      uint64_t field;
    } updated_;
    map<SteamSnowflake, SteamUser> friends_;
    map<SteamSnowflake, Image> userImages_;
    map<SteamSnowflake, SteamStats> userStats_;
    int globalPlayercount_;
  };

  class Steam {
  private:
    static Host* host_;
    static System* eng_;
    uint32_t appID_;
    SteamState state_;
    GameInstallationState installation_;
    utf8String commandLine_;
    struct Counters {
      StateUpdateIndex statsUpdate = 0;
    } counters_;
    STEAM_CALLBACK( Steam, OnGameOverlayActivated, GameOverlayActivated_t );
    STEAM_CALLBACK( Steam, OnIPCFailure, IPCFailure_t ); // respond by killing steam and reconnecting
    STEAM_CALLBACK( Steam, OnLicensesUpdated, LicensesUpdated_t );
    STEAM_CALLBACK( Steam, OnSteamServersConnected, SteamServersConnected_t );
    STEAM_CALLBACK( Steam, OnSteamServersDisconnected, SteamServersDisconnected_t );
    STEAM_CALLBACK( Steam, OnNewUrlLaunchParameters, NewUrlLaunchParameters_t );
    // Stats
    STEAM_CALLBACK( Steam, OnUserStatsReceived, UserStatsReceived_t );
    STEAM_CALLBACK( Steam, OnUserStatsStored, UserStatsStored_t );
    STEAM_CALLBACK( Steam, OnNumberOfCurrentPlayers, NumberOfCurrentPlayers_t );
    static void __cdecl staticSteamAPIWarningHook( int severity, const char* message );
  protected:
    void fetchImage( SteamUser& user );
  public:
    Steam( uint32_t applicationID, Host* host, System* engine );
    static void baseInitialize();
    static void baseShutdown();
    inline const GameInstallationState& installation() throw() { return installation_; }
    inline const utf8String& commandline() throw() { return commandLine_; }
    inline const SteamStats& myStats() const
    {
      const auto myid = state_.localUser_.id_.ConvertToUint64();
      return state_.userStats_.at( myid );
    }
    void refreshStats();
    void initialize();
    void update( double gameTime, double delta );
    void statAdd( const utf8String& name, int value );
    void statAdd( const utf8String& name, float value );
    void uploadStats();
    void shutdown();
    ~Steam();
  };

}