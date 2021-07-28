#include "stdafx.h"

#ifndef NEKO_NO_GUI

#include "MyGUI_NekoPlatform.h"
#include "gfx.h"

namespace MyGUI {

  NekoPlatform::NekoPlatform(): initialized_( false )
  {
    renderManager_ = make_unique<NekoRenderManager>();
    dataManager_ = make_unique<NekoDataManager>();
    logManager_ = make_unique<LogManager>();
  }

  NekoPlatform::~NekoPlatform()
  {
    assert( !initialized_ );
  }

  void NekoPlatform::initialise( neko::Gfx* gfx, const utf8String& logname )
  {
    assert( !initialized_ );
    initialized_ = true;

    LogManager::getInstance().setSTDOutputEnabled( false );
    LogManager::getInstance().setLoggingLevel( MyGUI::LogLevel::Info );

    if ( !logname.empty() )
      LogManager::getInstance().createDefaultSource( logname );

    renderManager_->initialise( &( gfx->renderer() ), gfx );
  }

  void NekoPlatform::shutdown()
  {
    assert( initialized_ );
    initialized_ = false;

    renderManager_->shutdown();
  }

}

#endif