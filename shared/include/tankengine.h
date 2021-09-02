#pragma once

#include "neko_types.h"

#define TANK_CALL __stdcall
#define TANK_EXPORT __declspec( dllexport )

namespace tank {

  using std::unique_ptr;
  using std::shared_ptr;
  using std::map;
  using std::move;
  using utf8String = neko::utf8String;

  using StateUpdateIndex = uint64_t;

  class Discord;
  class Steam;

  enum class InstallationHost
  {
    Local = 0,
    Steam,
    Discord
  };

  enum class GameOwnership
  {
    NotOwned = 0,
    Owned,
    TempFamilySharing,
    TempFreeWeekend
  };

  struct GameInstallationState
  {
    InstallationHost host_ = InstallationHost::Local;
    bool installed_ = false;
    utf8String branch_;
    utf8String installPath_;
    bool beta_ = false;
    GameOwnership ownership_ = GameOwnership::NotOwned;
    uint32_t purchaseTime_ = 0;
    uint64_t buildId_ = 0;
  };

  struct Image
  {
    size_t width_;
    size_t height_;
    std::vector<uint8_t> buffer_;
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

  class TankHost {
  public:
    virtual void onDiscordDebugPrint( const utf8String& message ) = 0;
    virtual void onDiscordUserImage( const DcSnowflake id, Image& image ) = 0;
    virtual void onSteamUserImage( const SteamSnowflake id, Image& image ) = 0;
    virtual void onSteamDebugPrint( const utf8String& message ) = 0;
    virtual void onSteamOverlayToggle( bool enabled ) = 0;
    virtual void onSteamStatsUpdated( StateUpdateIndex index ) = 0;
  };

  class TankEngine {
  private:
    unique_ptr<Discord> discord_;
    unique_ptr<Steam> steam_;
    TankHost* host_;
  public:
    TankEngine( TankHost* host );
    virtual void initialize( int64_t discordAppID, uint32_t steamAppID );
    virtual void changeActivity_AlphaDevelop() throw();
    virtual const GameInstallationState& localInstallation() throw();
    virtual const GameInstallationState& discordInstallation() throw();
    virtual const GameInstallationState& steamInstallation() throw();
    virtual void update( double gameTime, double delta );
    virtual void shutdown();
    virtual bool startedFromSteam();
    virtual bool startedFromDiscord();
    virtual void steamStatIncrement( const utf8String& name );
    virtual void steamStatAdd( const utf8String& name, int value );
    virtual void steamStatAdd( const utf8String& name, float value );
    virtual void uploadStats();
    virtual const map<utf8String, SteamStat>& steamStats();
    ~TankEngine();
  };

  using fnTankInitialize = TankEngine*( TANK_CALL* )( uint32_t version, TankHost* host );
  using fnTankShutdown = void( TANK_CALL* )( TankEngine* engine );

}