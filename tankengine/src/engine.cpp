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
      steam_ = std::make_unique<Steam>( steamAppID, host_, this );
      steam_->initialize();
    }
    if ( discordAppID )
    {
      discord_ = std::make_unique<Discord>( discordAppID, discordAppID, steamAppID, host_ );
      discord_->initialize();
    }
    if ( !startedFromSteam() && !startedFromDiscord() )
    {
      // Get current dir?
      // g_emptyGameInstallation.
    }
  }

  Account* TankEngine::accountBySteamID( SteamSnowflake id, bool create )
  {
    if ( me_.steamId_ == id )
      return &me_;
    for ( const auto& account : accounts_ )
    {
      if ( account.second && account.second->steamId_ == id )
        return account.second.get();
    }
    static uint64_t ctr = 0;
    if ( create )
    {
      ctr++;
      accounts_[ctr] = std::make_unique<Account>();
      auto ret = accounts_[ctr].get();
      ret->id_ = ctr;
      ret->steamId_ = id;
      return ret;
    }
    return nullptr;
  }

  Account* TankEngine::accountMe()
  {
    return &me_;
  }

  Account* TankEngine::account( uint64_t id )
  {
    if ( me_.id_ == id )
      return &me_;
    if ( accounts_.find( id ) != accounts_.end() )
      return accounts_[id].get();
    return nullptr;
  }

  void TankEngine::update( double gameTime, double delta )
  {
    if ( steam_ )
      steam_->update( gameTime, delta );
    if ( discord_ )
      discord_->update( gameTime, delta );
  }

  void TankEngine::changeActivity_AlphaDevelop() throw()
  {
    if ( discord_ )
      discord_->setActivityAlphaDevelop();
  }

  static GameInstallationState g_emptyGameInstallation;

  const GameInstallationState& TankEngine::localInstallation() throw()
  {
    return g_emptyGameInstallation;
  }

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

  bool TankEngine::startedFromSteam()
  {
    if ( !steam_ )
      return false;
    return ( !steam_->commandline().empty() );
  }

  bool TankEngine::startedFromDiscord()
  {
    if ( !discord_ )
      return false;
    return false;
  }

  void TankEngine::steamStatIncrement( const utf8String& name )
  {
    if ( steam_ )
      steam_->statAdd( name, 1 );
  }

  void TankEngine::steamStatAdd( const utf8String& name, int value )
  {
    if ( steam_ )
      steam_->statAdd( name, value );
  }

  void TankEngine::steamStatAdd( const utf8String& name, float value )
  {
    if ( steam_ )
      steam_->statAdd( name, value );
  }

  void TankEngine::uploadStats()
  {
    if ( steam_ )
      steam_->uploadStats();
  }

  const map<utf8String, SteamStat>& TankEngine::steamStats()
  {
    assert( steam_ );
    return steam_->myStats().map_;
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