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

  void Steam::baseInitialize()
  {
    if ( !SteamAPI_Init() )
      NEKO_EXCEPT( "Steam init failed" );
  }

  void Steam::baseShutdown()
  {
    SteamAPI_Shutdown();
  }

  void Steam::initialize()
  {
    SteamClient()->SetWarningMessageHook( staticSteamAPIWarningHook );

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

    char temp[1024] = { 0 };
    installation_.beta_ = SteamApps()->GetCurrentBetaName( temp, 1024 );
    installation_.branch_ = temp;

    memset( temp, 0, 1024 );
    SteamApps()->GetAppInstallDir( appID_, temp, 1024 );
    installation_.installPath_ = temp;
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