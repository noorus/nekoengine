#include "pch.h"
#include "steam.h"
#include "utilities.h"
#include "engine.h"

namespace neko {

  Steam* Steam::instance_ = nullptr;

  extern "C" void Steam::staticSteamAPIWarningHook( int severity, const char* message )
  {
    if ( instance_ && message )
      instance_->onSteamAPIWarning( severity, message );
  }

  const char* c_ownershipStrings[] = { "NotOwned", "Owned", "TempFamilySharing", "TempFreeWeekend" };

  Steam::Steam( EnginePtr engine, uint32_t appID ): Subsystem<Steam, LogSource::srcSteam>( engine ), appID_( appID )
  {
    instance_ = this;

    if ( !SteamAPI_Init() )
      return;

    SteamAPI_ManualDispatch_Init();

    utils()->SetWarningMessageHook( staticSteamAPIWarningHook );

    initialized_ = true;

    if ( user()->BLoggedOn() )
      onSteamConnected();

    recheckLaunchCommandline();

    auto sid = utils()->GetAppID();
    if ( sid != appID_ )
      NEKO_EXCEPT( "Steam returned wrong app ID" );

    localUser_.id_ = user()->GetSteamID();
    localUser_.name_ = friends()->GetFriendPersonaName( localUser_.id_ );
    localUser_.displayName_ = localUser_.name_;

    {
      if ( apps()->BIsSubscribed() )
        app_.ownership_ = GameOwnership::Owned;
      else if ( apps()->BIsSubscribedFromFamilySharing() )
        app_.ownership_ = GameOwnership::TempFamilySharing;
      else if ( apps()->BIsSubscribedFromFreeWeekend() )
        app_.ownership_ = GameOwnership::TempFreeWeekend;
      else
        app_.ownership_ = GameOwnership::NotOwned;
      app_.installed_ = apps()->BIsAppInstalled( appID_ );
      char temp[MAX_PATH] = { 0 };
      app_.beta_ = apps()->GetCurrentBetaName( temp, MAX_PATH );
      app_.branchName_ = temp;
      app_.buildID_ = apps()->GetAppBuildId();

      app_.purchaseTime_ = apps()->GetEarliestPurchaseUnixTime( appID_ );
    }

    log( "App ID: %i, Ownership: %s, Beta: %s, Branch: %s, Build ID: %i, Purchase time: %I64i", appID_,
      c_ownershipStrings[static_cast<int>(app_.ownership_)], app_.beta_ ? "yes" : "no", app_.branchName_.c_str(), app_.buildID_, app_.purchaseTime_ );

    log( "LocalUser: %s %s %s", utils::render( localUser_.id_ ).c_str(), localUser_.name_.c_str(),
      localUser_.displayName_.c_str() );
    
    refreshStats();
  }

  void Steam::onSteamAPIWarning( int severity, const char* message )
  {
    log( "Steam API warning severity %i: %s", severity, message );
  }

  void Steam::setState( SteamState newState )
  {
    auto oldState = state_;
    state_ = newState;
  }

  void Steam::recheckLaunchCommandline()
  {
    char tmp[1024] = { 0 };
    apps()->GetLaunchCommandLine( tmp, 1024 );
    utf8String cmdline = tmp;
    log( "Got launch cmdline: %s", cmdline.c_str() );
  }

  template <typename T>
  constexpr bool callEquals( const steam::CallbackMsg_t& callback )
  {
    return ( callback.m_iCallback == T::k_iCallback );
  }

  void Steam::runCallbacks()
  {
    auto pipe = steam::SteamAPI_GetHSteamPipe();
    SteamAPI_ManualDispatch_RunFrame( pipe );
    steam::CallbackMsg_t callback {};
    vector<uint8_t> buffer;
    while ( SteamAPI_ManualDispatch_GetNextCallback( pipe, &callback ) )
    {
      // Do not continue; or break; from these handlers,
      // the callback returned from the pipe must be freed

      if ( callEquals<steam::NewUrlLaunchParameters_t>( callback ) )
      {
        recheckLaunchCommandline();
      }
      else if ( callEquals<steam::UserStatsReceived_t>( callback ) )
      {
        onUserStatsReceived( reinterpret_cast<steam::UserStatsReceived_t*>( callback.m_pubParam ) );
      }
      else if ( callEquals<steam::GameOverlayActivated_t>( callback ) )
      {
        auto cb = reinterpret_cast<steam::GameOverlayActivated_t*>( callback.m_pubParam );
        if ( cb )
        {
          auto active = static_cast<uintptr_t>( cb->m_bActive );
          engine_->msgs()->send( M_Extern_SteamOverlay, 1, active );
        }
      }
      else if ( callEquals<steam::SteamAPICallCompleted_t>( callback ) )
      {
        auto completed = reinterpret_cast<steam::SteamAPICallCompleted_t*>( callback.m_pubParam );
        if ( completed )
        {
          if ( buffer.size() < completed->m_cubParam )
            buffer.resize( completed->m_cubParam );
          bool failed = true;
          if ( SteamAPI_ManualDispatch_GetAPICallResult(
                 pipe, completed->m_hAsyncCall, buffer.data(), completed->m_cubParam, completed->m_iCallback, &failed ) )
          {
            if ( failed )
              log( "Steam API async call returned failed, ignoring" );
            else
            {
              if ( asyncCalls_.find( completed->m_hAsyncCall ) == asyncCalls_.end() )
                log( "Got unknown SteamAPI AsyncCall APICallResult" );
              else
              {
                auto type = asyncCalls_.at( completed->m_hAsyncCall );
                if ( type == CallType_NumberOfCurrentPlayers )
                  onNumberOfCurrentPlayers( reinterpret_cast<steam::NumberOfCurrentPlayers_t*>( buffer.data() ) );
              }
            }
          }
        }
      }
      else if ( callEquals<steam::SteamServerConnectFailure_t>( callback ) )
      {
        auto cb = reinterpret_cast<steam::SteamServerConnectFailure_t*>( callback.m_pubParam );
        if ( cb )
          log( "Failed to connect to Steam (%s)%s", utils::render( cb->m_eResult ).c_str(),
            cb->m_bStillRetrying ? "" : " (still retrying)" );
        setState( Steam_Disconnected );
      }
      else if ( callEquals<steam::SteamServersConnected_t>( callback ) )
      {
        onSteamConnected();
      }
      else if ( callEquals<steam::SteamServersDisconnected_t>( callback ) )
      {
        onSteamDisconnected( reinterpret_cast<steam::SteamServersDisconnected_t*>( callback.m_pubParam ) );
      }

      SteamAPI_ManualDispatch_FreeLastCallback( pipe );
    }
  }

  void Steam::onSteamConnected()
  {
    // auto cb = reinterpret_cast<steam::SteamServersConnected_t*>( callback.m_pubParam );
    log( "Connected to Steam successfully" );
    setState( Steam_Connected );
    log( "My SteamID is %s (%I64u)", utils::render( user()->GetSteamID() ).c_str(), user()->GetSteamID().ConvertToUint64() );
  }

  void Steam::onSteamDisconnected( steam::SteamServersDisconnected_t* result )
  {
    log( "Disconnected from Steam (%s)", result ? utils::render( result->m_eResult ).c_str() : "No result code" );
    setState( Steam_Disconnected );
  }

  const std::vector<std::pair<utf8String, StatType>> c_statDefinitions = {
    { "dev_launches", StatType::Int },
    { "dev_debugTime", StatType::Float }
  };

  void Steam::onUserStatsReceived( steam::UserStatsReceived_t* cb )
  {
    if ( cb->m_eResult != steam::k_EResultOK )
      return;
    if ( cb->m_nGameID != appID_ )
      return;
    log( "Received stats" );
    SteamStats sts;
    sts.id_ = cb->m_steamIDUser.ConvertToUint64();
    int ret = 0;
    for ( const auto& entry : c_statDefinitions )
    {
      SteamStat stat;
      stat.name_ = entry.first;
      stat.type_ = entry.second;
      if ( sts.id_ == localUser_.id_.ConvertToUint64() )
      {
        if ( stat.type_ == StatType::Int )
          ret += stats()->GetStat( stat.name_.c_str(), &stat.i_ ) ? 1 : 0;
        else if ( stat.type_ == StatType::Float )
          ret += stats()->GetStat( stat.name_.c_str(), &stat.f_ ) ? 1 : 0;
      }
      else
      {
        if ( stat.type_ == StatType::Int )
          ret += stats()->GetUserStat( cb->m_steamIDUser, stat.name_.c_str(), &stat.i_ ) ? 1 : 0;
        else if ( stat.type_ == StatType::Float )
          ret += stats()->GetUserStat( cb->m_steamIDUser, stat.name_.c_str(), &stat.f_ ) ? 1 : 0;
      }
      sts.map_[entry.first] = move( stat );
    }
    if ( ret )
      userStats_[sts.id_] = move( sts );
  }

  void Steam::onNumberOfCurrentPlayers( steam::NumberOfCurrentPlayers_t* result )
  {
    if ( result->m_bSuccess )
    {
      globalPlayercount_ = result->m_cPlayers;
      log( "onNumberOfCurrentPlayers: %i", *globalPlayercount_ );
    }
  }

  void Steam::refreshStats()
  {
    log( "Refreshing stats" );
    stats()->RequestCurrentStats();
    asyncCalls_[stats()->GetNumberOfCurrentPlayers()] = CallType_NumberOfCurrentPlayers;
  }

  void Steam::statAdd( const utf8String& name, int value )
  {
    auto sts = myStats();
    if ( !sts )
      return;
    auto val = sts->map_.at( name ).i_;
    stats()->SetStat( name.c_str(), val + value );
  }

  void Steam::statAdd( const utf8String& name, float value )
  {
    auto sts = myStats();
    if ( !sts )
      return;
    auto val = sts->map_.at( name ).f_;
    stats()->SetStat( name.c_str(), val + value );
  }

  void Steam::uploadStats()
  {
    if ( !stats() )
      return;
    log( "Uploading stats" );
    stats()->StoreStats();
  }

  void Steam::preUpdate( GameTime time )
  {
    //
  }

  void Steam::tick( GameTime tick, GameTime time )
  {
    if ( !initialized_ )
      return;

    runCallbacks();
  }

  void Steam::postUpdate( GameTime delta, GameTime tick )
  {
    //
  }

  Steam::~Steam()
  {
    if ( initialized_ )
      SteamAPI_Shutdown();
  }

}