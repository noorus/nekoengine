#include "stdafx.h"
#include "steam.h"
#include "neko_exception.h"

#include "steam/steam_api.h"
#include "steam/isteamclient.h"

namespace tank {

  TankHost* Steam::host_ = nullptr;
  TankEngine* Steam::eng_ = nullptr;

  Steam::Steam( uint32_t applicationID, TankHost* host, TankEngine* engine ):
  appID_( applicationID )
  {
    host_ = host;
    eng_ = engine;
  }

  extern "C" void Steam::staticSteamAPIWarningHook( int severity, const char* message )
  {
    if ( Steam::host_ )
    {
      host_->onSteamDebugPrint( message );
    }
  }

  void Steam::OnGameOverlayActivated( GameOverlayActivated_t* cb )
  {
    host_->onSteamOverlayToggle( cb->m_bActive );
  }

  void Steam::OnIPCFailure( IPCFailure_t* cb )
  {
    host_->onSteamDebugPrint( "IPC OnIPCFailure" );
  }

  void Steam::OnSteamServersConnected( SteamServersConnected_t* cb )
  {
    host_->onSteamDebugPrint( "OnSteamServersConnected" );
  }

  void Steam::OnSteamServersDisconnected( SteamServersDisconnected_t* cb )
  {
    host_->onSteamDebugPrint( "OnSteamServersDisconnected" );
  }

  void Steam::OnLicensesUpdated( LicensesUpdated_t* cb )
  {
    host_->onSteamDebugPrint( "OnLicensesUpdated" );
  }

  void Steam::OnNewUrlLaunchParameters( NewUrlLaunchParameters_t* cb )
  {
    host_->onSteamDebugPrint( "OnNewUrlLaunchParameters" );
    char cmdline[1024] = { 0 };
    SteamApps()->GetLaunchCommandLine( cmdline, 1024 );
    host_->onSteamDebugPrint( cmdline );
  }

  // Stats

  const std::vector<std::pair<utf8String, StatType>> c_statDefinitions = {
    { "dev_launches", StatType::Int },
    { "dev_debugTime", StatType::Float }
  };

  void Steam::OnUserStatsReceived( UserStatsReceived_t* cb )
  {
    if ( cb->m_eResult != k_EResultOK )
      return;
    if ( cb->m_nGameID != appID_ )
      return;
    SteamStats stats;
    stats.id_ = cb->m_steamIDUser.ConvertToUint64();
    int ret = 0;
    for ( const auto& entry : c_statDefinitions )
    {
      SteamStat stat;
      stat.name_ = entry.first;
      stat.type_ = entry.second;
      if ( stats.id_ == state_.localUser_.id_.ConvertToUint64() )
      {
        if ( stat.type_ == StatType::Int )
          ret += SteamUserStats()->GetStat( stat.name_.c_str(), &stat.i_ ) ? 1 : 0;
        else if ( stat.type_ == StatType::Float )
          ret += SteamUserStats()->GetStat( stat.name_.c_str(), &stat.f_ ) ? 1 : 0;
      }
      else
      {
        if ( stat.type_ == StatType::Int )
          ret += SteamUserStats()->GetUserStat( cb->m_steamIDUser, stat.name_.c_str(), &stat.i_ ) ? 1 : 0;
        else if ( stat.type_ == StatType::Float )
          ret += SteamUserStats()->GetUserStat( cb->m_steamIDUser, stat.name_.c_str(), &stat.f_ ) ? 1 : 0;
      }
      stats.map_[entry.first] = move( stat );
    }
    if ( ret )
    {
      state_.userStats_[stats.id_] = move( stats );
      state_.updated_.bits.stats = true;
    }
  }

  void Steam::OnUserStatsStored( UserStatsStored_t* cb )
  {
  }

  void Steam::OnNumberOfCurrentPlayers( NumberOfCurrentPlayers_t* cb )
  {
    if ( !cb->m_bSuccess )
      return;
    state_.globalPlayercount_ = cb->m_cPlayers;
    state_.updated_.bits.playerCount = true;
  }

  // Class impl

  void Steam::baseInitialize()
  {
    if ( !SteamAPI_Init() )
      NEKO_EXCEPT( "Steam init failed" );
  }

  void Steam::baseShutdown()
  {
    SteamAPI_Shutdown();
  }

  void getSteamUser( CSteamID from, SteamUser& to )
  {
    to.id_ = from;
    to.name_ = SteamFriends()->GetFriendPersonaName( from );
    auto nickname = SteamFriends()->GetPlayerNickname( from );
    to.displayName_ = ( nickname ? nickname : to.name_ );
  }

  void Steam::fetchImage( SteamUser& user )
  {
    auto i = ::SteamFriends()->GetLargeFriendAvatar( user.id_ );
    uint32_t width, height;
    if ( !SteamUtils()->GetImageSize( i, &width, &height ) )
      return;
    Image img;
    img.width_ = width;
    img.height_ = height;
    size_t bytes = img.width_ * img.height_ * 4;
    img.buffer_.resize( bytes );
    auto buffer = img.buffer_.data();
    if ( !SteamUtils()->GetImageRGBA( i, buffer, static_cast<int>( bytes ) ) )
      return;
    auto acc = user.id_.ConvertToUint64();
    state_.userImages_[acc] = move( img );
    state_.updated_.bits.images = true;
  }

  void Steam::refreshStats()
  {
    ::SteamUserStats()->RequestCurrentStats();
    ::SteamUserStats()->GetNumberOfCurrentPlayers();
  }

  void Steam::statAdd( const utf8String& name, int value )
  {
    auto val = myStats().map_.at( name ).i_;
    ::SteamUserStats()->SetStat( name.c_str(), val + value );
  }

  void Steam::statAdd( const utf8String& name, float value )
  {
    auto val = myStats().map_.at( name ).f_;
    ::SteamUserStats()->SetStat( name.c_str(), val + value );
  }

  void Steam::uploadStats()
  {
    ::SteamUserStats()->StoreStats();
  }

  void Steam::initialize()
  {
    ::SteamClient()->SetWarningMessageHook( staticSteamAPIWarningHook );

    state_.localUser_.id_ = ::SteamUser()->GetSteamID();
    state_.localUser_.name_ = SteamFriends()->GetFriendPersonaName( state_.localUser_.id_ );
    state_.localUser_.displayName_ = state_.localUser_.name_;
    state_.updated_.bits.self = true;

    installation_.host_ = InstallationHost::Steam;

    char cmdline[1024] = { 0 };
    SteamApps()->GetLaunchCommandLine( cmdline, 1024 );
    commandLine_ = cmdline;

    if ( SteamApps()->BIsSubscribed() )
      installation_.ownership_ = GameOwnership::Owned;
    else if ( SteamApps()->BIsSubscribedFromFamilySharing() )
      installation_.ownership_ = GameOwnership::TempFamilySharing;
    else if ( SteamApps()->BIsSubscribedFromFreeWeekend() )
      installation_.ownership_ = GameOwnership::TempFreeWeekend;
    else
      installation_.ownership_ = GameOwnership::NotOwned;

    installation_.installed_ = SteamApps()->BIsAppInstalled( appID_ );

    installation_.buildId_ = SteamApps()->GetAppBuildId();

    installation_.purchaseTime_ = SteamApps()->GetEarliestPurchaseUnixTime( appID_ );

    char temp[MAX_PATH] = { 0 };
    installation_.beta_ = SteamApps()->GetCurrentBetaName( temp, MAX_PATH );
    installation_.branch_ = temp;

    memset( temp, 0, MAX_PATH );
    SteamApps()->GetAppInstallDir( appID_, temp, MAX_PATH );
    installation_.installPath_ = temp;

    refreshStats();

    fetchImage( state_.localUser_ );
  }

  void Steam::update( double gameTime, double delta )
  {
    SteamAPI_RunCallbacks();

    char asd[1024];

    if ( state_.updated_.field )
    {
      if ( state_.updated_.bits.self )
      {
        sprintf_s( asd, 1024, "self: %s (%I64u)", state_.localUser_.name_.c_str(), state_.localUser_.id_.ConvertToUint64() );
        host_->onSteamDebugPrint( asd );
        auto acc = eng_->accountMe();
        acc->steamId_ = state_.localUser_.id_.ConvertToUint64();
        acc->steamName_ = state_.localUser_.name_;
        host_->onAccountUpdated( *acc );
        fetchImage( state_.localUser_ );
      }
      if ( state_.updated_.bits.friends )
      {
        for ( auto& frnd : state_.friends_ )
        {
          sprintf_s( asd, 1024, "friend: %s (%I64u)", frnd.second.name_.c_str(), frnd.second.id_.ConvertToUint64() );
          host_->onSteamDebugPrint( asd );
        }
      }
      if ( state_.updated_.bits.images )
      {
        for ( auto& img : state_.userImages_ )
        {
          auto acc = eng_->accountBySteamID( img.first, true );
          if ( !acc->steamImage_ )
          {
            acc->steamImage_ = std::make_unique<Image>();
            *acc->steamImage_ = img.second;
            host_->onAccountUpdated( *acc );
          }
        }
      }
      if ( state_.updated_.bits.stats )
      {
        for ( auto& stats : state_.userStats_ )
        {
          for ( auto& stat : stats.second.map_ )
          {
            sprintf_s( asd, 1024, "stat: uid %I64u stat %s val %i %f", stats.first, stat.second.name_.c_str(), stat.second.i_, stat.second.f_ );
            host_->onSteamDebugPrint( asd );
          }
        }
        host_->onSteamStatsUpdated( counters_.statsUpdate++ );
      }
      if ( state_.updated_.bits.playerCount )
      {
        sprintf_s( asd, 1024, "player count: %i", state_.globalPlayercount_ );
        host_->onSteamDebugPrint( asd );
      }
      state_.updated_.field = 0;
    }
  }

  void Steam::shutdown()
  {
  }

  Steam::~Steam()
  {
    host_ = nullptr;
  }

}