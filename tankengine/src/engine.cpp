#include "stdafx.h"
#include "tankengine.h"
#include "discord.h"
#include "steam.h"

namespace tank {

  TankEngine::TankEngine( TankHost* host ): host_( host )
  {
  }

  void TankEngine::initialize( int64_t discordAppID, uint32_t steamAppID )
  {
    if ( steamAppID )
    {
      Steam::baseInitialize();
      steam_ = std::make_unique<Steam>( steamAppID, host_ );
      steam_->initialize();
    }
    if ( discordAppID )
    {
      discord_ = std::make_unique<Discord>( discordAppID, discordAppID, steamAppID, host_ );
      discord_->initialize();
    }
  }

  void TankEngine::update( double gameTime, double delta )
  {
    if ( steam_ )
      steam_->update();
    if ( discord_ )
      discord_->update( gameTime, delta );
  }

  void TankEngine::changeActivity_AlphaDevelop() throw()
  {
    if ( discord_ )
      discord_->setActivityAlphaDevelop();
  }

  static GameInstallationState g_emptyGameInstallation;

  const GameInstallationState& TankEngine::discordInstallation() throw()
  {
    if ( !discord_ )
      return g_emptyGameInstallation;

    return discord_->installation();
  }

  const GameInstallationState& TankEngine::steamInstallation() throw()
  {
    if ( !steam_ )
      return g_emptyGameInstallation;

    return steam_->installation();
  }

  void TankEngine::shutdown()
  {
    if ( discord_ )
    {
      discord_->shutdown();
      // discord_.reset();
    }
    if ( steam_ )
    {
      steam_->shutdown();
      Steam::baseShutdown();
      steam_.reset();
    }
  }

  TankEngine::~TankEngine()
  {
    //
  }

}