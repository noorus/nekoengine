#include "pch.h"
#include "rainet.h"
#include "rainet_discord.h"
#include "rainet_steam.h"

namespace rainet {

  static GameInstallationState g_localGameInstallation;

  System::System( Host* host ): host_( host )
  {
  }

  void System::initialize( int64_t discordAppID, uint32_t steamAppID )
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
    if ( !startedFromSteam() )
    {
      g_localGameInstallation.host_ = InstallationHost::Local;
#ifdef _DEBUG
      g_localGameInstallation.beta_ = true;
      g_localGameInstallation.branch_ = "local-debug";
#else
      g_localGameInstallation.beta_ = false;
      g_localGameInstallation.branch_ = "local-release";
#endif
      g_localGameInstallation.ownership_ = GameOwnership::Owned;
      g_localGameInstallation.buildId_ = 1;
      g_localGameInstallation.purchaseTime_ = 0;
      g_localGameInstallation.installed_ = true;
      g_localGameInstallation.installPath_ = neko::platform::wideToUtf8( neko::platform::getCurrentDirectory() );
    }
  }

  Account* System::accountBySteamID( SteamSnowflake id, bool create )
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

  Account* System::accountMe()
  {
    return &me_;
  }

  Account* System::account( uint64_t id )
  {
    if ( me_.id_ == id )
      return &me_;
    if ( accounts_.find( id ) != accounts_.end() )
      return accounts_[id].get();
    return nullptr;
  }

  void System::update( double gameTime, double delta )
  {
    if ( steam_ )
      steam_->update( gameTime, delta );
    if ( discord_ )
      discord_->update( gameTime, delta );
  }

  void System::changeActivity_AlphaDevelop() throw()
  {
    if ( discord_ )
      discord_->setActivityAlphaDevelop();
  }

  const GameInstallationState& System::localInstallation() throw()
  {
    return g_localGameInstallation;
  }

  const GameInstallationState& System::steamInstallation() throw()
  {
    if ( !steam_ )
      return g_localGameInstallation;

    return steam_->installation();
  }

  bool System::startedFromSteam()
  {
    if ( !steam_ )
      return false;
    return ( !steam_->commandline().empty() );
  }

  void System::statIncrement( const utf8String& name )
  {
    if ( steam_ )
      steam_->statAdd( name, 1 );
  }

  void System::statAdd( const utf8String& name, int value )
  {
    if ( steam_ )
      steam_->statAdd( name, value );
  }

  void System::statAdd( const utf8String& name, float value )
  {
    if ( steam_ )
      steam_->statAdd( name, value );
  }

  void System::uploadStats()
  {
    if ( steam_ )
      steam_->uploadStats();
  }

  const map<utf8String, SteamStat>& System::steamStats()
  {
    assert( steam_ );
    return steam_->myStats().map_;
  }

  void System::shutdown()
  {
    if ( discord_ )
    {
      discord_->shutdown();
      discord_.reset();
    }
    if ( steam_ )
    {
      steam_->shutdown();
      Steam::baseShutdown();
      steam_.reset();
    }
  }

  System::~System()
  {
    //
  }

}