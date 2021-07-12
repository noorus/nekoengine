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
    discord_ = std::make_unique<Discord>( discordAppID, discordAppID, steamAppID, host_ );
    discord_->initialize();

    Steam::baseInitialize();
    steam_ = std::make_unique<Steam>( steamAppID, host_ );
    steam_->initialize();
  }

  void TankEngine::update()
  {
    steam_->update();
    discord_->update();
  }

  const GameInstallationState& TankEngine::discordInstallation() throw()
  {
    return discord_->installation();
  }

  const GameInstallationState& TankEngine::steamInstallation() throw()
  {
    return steam_->installation();
  }

  void TankEngine::shutdown()
  {
    if ( steam_ )
    {
      steam_->shutdown();
    }
    if ( discord_ )
    {
      discord_->shutdown();
      // discord_.reset();
    }
    Steam::baseShutdown();
  }

  TankEngine::~TankEngine()
  {
    //
  }

}