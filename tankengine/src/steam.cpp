#include "stdafx.h"
#include "steam.h"
#include "neko_exception.h"

#include "steam/steam_api.h"
#include "steam/isteamclient.h"

namespace tank {

  TankHost* Steam::host_ = nullptr;

  Steam::Steam( uint32_t applicationID, TankHost* host ):
    appID_( applicationID )
  {
    host_ = host;
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
    host_->onSteamDebugPrint( "OnLicensesUpdated" );
    char cmdline[1024] = { 0 };
    SteamApps()->GetLaunchCommandLine( cmdline, 1024 );
    host_->onSteamDebugPrint( cmdline );
  }

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
    uint32_t bytes = img.width_ * img.height_ * 4;
    img.buffer_.resize( bytes );
    auto buffer = img.buffer_.data();
    if ( !SteamUtils()->GetImageRGBA( i, buffer, bytes ) )
      return;
    auto acc = user.id_.ConvertToUint64();
    state_.userImages_[acc] = move( img );
    state_.updated_.bits.images = true;
    host_->onSteamUserImage( acc, state_.userImages_[acc] );
  }

  void Steam::initialize()
  {
    SteamClient()->SetWarningMessageHook( staticSteamAPIWarningHook );

    state_.localUser_.id_ = ::SteamUser()->GetSteamID();
    state_.localUser_.name_ = SteamFriends()->GetFriendPersonaName( state_.localUser_.id_ );
    state_.localUser_.displayName_ = state_.localUser_.name_;

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

    fetchImage( state_.localUser_ );
  }

  void Steam::update()
  {
    SteamAPI_RunCallbacks();
  }

  void Steam::shutdown()
  {
  }

  Steam::~Steam()
  {
    host_ = nullptr;
  }

}