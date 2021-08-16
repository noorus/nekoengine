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

  class TankHost {
  public:
    virtual void onDiscordDebugPrint( const utf8String& message ) = 0;
    virtual void onDiscordUserImage( const uint64_t userId, size_t width, size_t height, const uint8_t* data ) = 0;
    virtual void onSteamDebugPrint( const utf8String& message ) = 0;
    virtual void onSteamOverlayToggle( bool enabled ) = 0;
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
    virtual const GameInstallationState& discordInstallation() throw();
    virtual const GameInstallationState& steamInstallation() throw();
    virtual void update( double gameTime, double delta );
    virtual void shutdown();
    ~TankEngine();
  };

  using fnTankInitialize = TankEngine*( TANK_CALL* )( uint32_t version, TankHost* host );
  using fnTankShutdown = void( TANK_CALL* )( TankEngine* engine );

}