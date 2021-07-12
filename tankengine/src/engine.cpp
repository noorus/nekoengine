#include "stdafx.h"
#include "tankengine.h"
#include "discord.h"

namespace tank {

  TankEngine::TankEngine( TankHost* host ): host_( host )
  {
  }

  void TankEngine::initialize( int64_t discordAppID, uint32_t steamAppID )
  {
    discord_ = std::make_unique<Discord>( discordAppID, discordAppID, steamAppID, host_ );
    discord_->initialize();
  }

  void TankEngine::update()
  {
    discord_->update();
  }

  void TankEngine::shutdown()
  {
    if ( discord_ )
    {
      discord_->shutdown();
      // discord_.reset();
    }
  }

  TankEngine::~TankEngine()
  {
    //
  }

}