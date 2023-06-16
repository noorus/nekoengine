#pragma once
#include "pch.h"
#include "neatlywrappedsteamapi.h"
#include "forwards.h"

namespace neko {

  enum class GameOwnership
  {
    NotOwned = 0,
    Owned,
    TempFamilySharing,
    TempFreeWeekend
  };

  struct AppState
  {
    GameOwnership ownership_ = GameOwnership::NotOwned;
    bool installed_ = false;
    bool beta_ = false;
    uint32_t purchaseTime_ = 0;
    uint64_t buildID_ = 0;
    utf8String branchName_;
  };

  using SteamSnowflake = uint64_t;
  using DcSnowflake = uint64_t;

  enum class StatType
  {
    Unknown = 0,
    Int,
    Float
  };

  struct SteamStat
  {
    utf8String name_;
    StatType type_ = StatType::Unknown;
    int i_ = 0;
    float f_ = 0.0f;
  };

  struct SteamStats
  {
    SteamSnowflake id_ = 0;
    map<utf8String, SteamStat> map_;
  };

  struct Account
  {
    uint64_t id_;
    SteamSnowflake steamId_;
    utf8String steamName_;
    unique_ptr<Image> steamImage_;
  };

  struct SteamUser
  {
    steam::CSteamID id_;
    utf8String name_;
    utf8String displayName_;
  };

  class Steam: public Subsystem<Steam, srcSteam> {
  public:
    enum SteamState
    {
      Steam_Disconnected = 0,
      Steam_Login,
      Steam_Connected
    };
    enum SteamAPICallType
    {
      CallType_NumberOfCurrentPlayers
    };
  protected:
    static Steam* instance_;
    bool initialized_ = false;
    SteamState state_ = Steam_Disconnected;
    uint32_t appID_ = 0;
    AppState app_;
    SteamUser localUser_;
    map<SteamSnowflake, SteamUser> friends_;
    map<SteamSnowflake, SteamStats> userStats_;
    optional<int> globalPlayercount_ = 0;
    static void __cdecl staticSteamAPIWarningHook( int severity, const char* message );
    void onSteamAPIWarning( int severity, const char* message );
    void onSteamConnected();
    void onSteamDisconnected( steam::SteamServersDisconnected_t* result );
    void onUserStatsReceived( steam::UserStatsReceived_t* result );
    void onNumberOfCurrentPlayers( steam::NumberOfCurrentPlayers_t* result );
    map<steam::SteamAPICall_t, SteamAPICallType> asyncCalls_;
    void runCallbacks();
    void recheckLaunchCommandline();
    void setState( SteamState newState );
  public:
    Steam( EnginePtr engine, uint32_t appID );
    void preUpdate( GameTime time ) override;
    void tick( GameTime tick, GameTime time ) override;
    void postUpdate( GameTime delta, GameTime tick ) override;
    inline const SteamUser& me() const noexcept { return localUser_; }
    inline const SteamStats* myStats() const
    {
      const auto myid = localUser_.id_.ConvertToUint64();
      if ( !myid || userStats_.find( myid ) == userStats_.end() )
        return nullptr;
      return &userStats_.at( myid );
    }
    void refreshStats();
    void statAdd( const utf8String& name, int value );
    void statAdd( const utf8String& name, float value );
    void uploadStats();
    inline const AppState& state() { return app_; }
    inline steam::ISteamApps* apps() { return steam::SteamApps(); }
    inline steam::ISteamUser* user() { return steam::SteamUser(); }
    inline steam::ISteamFriends* friends() { return steam::SteamFriends(); }
    inline steam::ISteamUserStats* stats() { return steam::SteamUserStats(); }
    inline steam::ISteamClient* client() { return steam::SteamClient(); }
    inline steam::ISteamUtils* utils() { return steam::SteamUtils(); }
    inline steam::ISteamInventory* inventory() { return steam::SteamInventory(); }
    virtual ~Steam();
  };

}