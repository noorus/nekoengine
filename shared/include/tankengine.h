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

  class TankHost {
  public:
    virtual void logPrint( const utf8String& message ) = 0;
  };

  class TankEngine {
  private:
    unique_ptr<Discord> discord_;
    TankHost* host_;
  public:
    TankEngine( TankHost* host );
    virtual void initialize( int64_t discordAppID, uint32_t steamAppID );
    virtual void update();
    virtual void shutdown();
    ~TankEngine();
  };

  using fnTankInitialize = TankEngine*( TANK_CALL* )( uint32_t version, TankHost* host );
  using fnTankShutdown = void( TANK_CALL* )( TankEngine* engine );

}