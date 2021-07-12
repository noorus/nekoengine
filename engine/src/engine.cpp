#pragma once
#include "stdafx.h"
#include "engine.h"
#include "console.h"
#include "neko_exception.h"
#include "gfx.h"
#include "scripting.h"
#include "loader.h"
#include "messaging.h"
#include "director.h"

namespace neko {

  GameTime c_logicFPS = 60.0; //!< 60 fps
  GameTime c_logicStep = ( 1.0 / c_logicFPS ); //!< tick ms
  uint64_t c_logicMaxFrameMicroseconds = static_cast<uint64_t>( ( c_logicStep * 1000.0 ) * 1000.0 ); //!< max us

  const char* c_engineName = "Nekoengine Alpha";
  const char* c_engineLogName = "nekoengine";
  const uint32_t c_engineVersion[3] = { 0, 1, 1 };

  const int64_t c_discordAppId = 862843623824556052;
  const uint32_t c_steamAppId = 1692910;

#ifdef _DEBUG
  const wchar_t* c_tankLibraryName = L"tankengine_d.dll";
#else
  const wchar_t* c_tankLibraryName = L"tankengine.dll";
#endif

  Engine::Engine( ConsolePtr console ):
  console_( move( console ) ), time_( 0.0 ), signal_( Signal_None )
  {
    info_.logName = c_engineLogName;
    info_.engineName = c_engineName;
    info_.profile = _PROFILE;
    info_.compiled = __DATE__ " " __TIME__;
    info_.compiler = _COMPILER;
    info_.major = c_engineVersion[0];
    info_.minor = c_engineVersion[1];
    info_.build = c_engineVersion[2];
  }

  Engine::~Engine()
  {
    shutdown();
  }

  void TankLibrary::load( tank::TankHost* host )
  {
    module_ = LoadLibraryW( c_tankLibraryName );
    if ( !module_ )
      NEKO_EXCEPT( "TankEngine load failed" );
    pfnTankInitialize = reinterpret_cast<tank::fnTankInitialize>( GetProcAddress( module_, "tankInitialize" ) );
    pfnTankShutdown = reinterpret_cast<tank::fnTankShutdown>( GetProcAddress( module_, "tankShutdown" ) );
    if ( !pfnTankInitialize || !pfnTankShutdown )
      NEKO_EXCEPT( "TankEngine export resolution failed" );
    engine_ = pfnTankInitialize( 1, host );
    if ( !engine_ )
      NEKO_EXCEPT( "TankEngine init failed" );
  }

  void TankLibrary::unload()
  {
    if ( engine_ && pfnTankShutdown )
      pfnTankShutdown( engine_ );
    // if ( module_ )
    //   FreeLibrary( module_ );
  }

  void Engine::initialize( const Options& options )
  {
    console_->setEngine( shared_from_this() );

    platform::PerformanceTimer timer;

    UVersionInfo icuVersion;
    u_getVersion( icuVersion );
    console_->printf( Console::srcEngine, "Using ICU version %d.%d.%d", icuVersion[0], icuVersion[1], icuVersion[2] );

    tanklib_.load( this );
    tanklib_.engine_->initialize( c_discordAppId, c_steamAppId );

    loader_ = make_shared<ThreadedLoader>();
    loader_->start();

    messaging_ = make_shared<Messaging>( shared_from_this() );
    Locator::provideMessaging( messaging_ ); // WARN Technically a bad pattern, but fine as long as there's one Engine
    messaging_->listen( this );

    director_ = make_shared<Director>();
    director_->reset();
    Locator::provideDirector( director_ );

    timer.start();
    fonts_ = make_shared<FontManager>( shared_from_this() );
    fonts_->initialize();
    console_->printf( Console::srcFonts, "Font manager init took %dms", (int)timer.stop() );

    timer.start();
    renderer_ = make_shared<ThreadedRenderer>( loader_, fonts_, messaging_, director_, console_ );
    renderer_->start();
    console_->printf( Console::srcGfx, "Renderer init took %dms", (int)timer.stop() );

    timer.start();
    scripting_ = make_shared<Scripting>( shared_from_this() );
    scripting_->initialize();
    console_->printf( Console::srcScripting, "Scripting init took %dms", (int)timer.stop() );

    scripting_->postInitialize();

    tanklib_.engine_->update();
  }

  void Engine::triggerFatalError( FatalError error )
  {
    signal_ = Signal_Stop;
  }

  void Engine::onMessage( const Message& msg )
  {
    switch ( msg.code )
    {
      case M_Window_LostFocus:
        state_.focusLost = true;
        break;
      case M_Window_GainedFocus:
        state_.focusLost = false;
        break;
      case M_Window_EnterMove:
        state_.windowMove = true;
        break;
      case M_Window_ExitMove:
        state_.windowMove = false;
        break;
      case M_Window_Close:
        signal_ = Signal_Stop;
        break;
      case M_Debug_ReloadScript:
        signal_ = Signal_Restart;
        break;
    }
  }

  void Engine::onDiscordDebugPrint( const utf8String& message )
  {
    console_->printf( Console::srcEngine, "Discord: %s", message.c_str() );
  }

  void Engine::onSteamDebugPrint( const utf8String& message )
  {
    console_->printf( Console::srcEngine, "Steam: %s", message.c_str() );
  }

  void Engine::onSteamOverlayToggle( bool enabled )
  {
    console_->printf( Console::srcEngine, "Steam: Overlay toggle %s", enabled ? "true" : "false" );
  }

  bool Engine::paused()
  {
    if ( state_.focusLost || state_.windowMove )
      return true;
    return false;
  }

  void Engine::restart()
  {
    scripting_->shutdown();
    renderer_->restart();
    scripting_->initialize();
    scripting_->postInitialize();
  }

  void Engine::run()
  {
    clock_.init();
    time_ = 0.0;

    GameTime accumulator = 0.0;
    GameTime delta = 0.0;

    const auto maxSleepytimeMs = math::ifloor( (double)c_logicMaxFrameMicroseconds / 1000.0 );

    console_->printf( Console::srcEngine, "Logic: targeting %.02f FPS, logic step %.02fms, max frame time %I64uus, max sleep %ims",
      (float)c_logicFPS, static_cast<float>( c_logicStep * 1000.0 ), c_logicMaxFrameMicroseconds, maxSleepytimeMs );

    tanklib_.engine_->update();

    auto& inst = tanklib_.engine_->steamInstallation();

    while ( signal_ != Signal_Stop )
    {
      delta = clock_.update();

      console_->executeBuffered();

      //gfx_->processEvents();
      messaging_->processEvents();

      if ( signal_ == Signal_Restart )
      {
        signal_ = Signal_None;
        restart();
        time_ = 0.0;
        accumulator = 0.0;
        clock_.update();
        continue;
      }

      scripting_->preUpdate( time_ );
      fonts_->prepare( time_ );
      //gfx_->preUpdate( time_ );

      if ( !paused() )
      {
        accumulator += delta;
        while ( accumulator >= c_logicStep )
        {
          scripting_->tick( c_logicStep, time_ );
          messaging_->tick( c_logicStep, time_ );
          //gfx_->tick( cLogicStep, time_ );
          time_ += c_logicStep;
          accumulator -= c_logicStep;
        }
      }

      tanklib_.engine_->update();

      scripting_->postUpdate( delta, time_ );

      if ( delta > 0.0 && signal_ != Signal_Stop )
      {
        //gfx_->postUpdate( delta, time_ );
      }

      auto us = clock_.peekMicroseconds();
      if ( us >= c_logicMaxFrameMicroseconds )
      {
        console_->printf( Console::srcEngine, "WARNING: Logic frame exceeded max time (%I64uus)", us );
      }
      else
      {
        /* this pretty sleep math is for if you really want to save on CPU, but makes for some jitter.
        auto ms = math::ifloor( static_cast<double>( cLogicMaxFrameMicroseconds - us ) / 1000.0 );
        static char asd[64];
        sprintf_s( asd, 64, "sleeping for %i ms\r\n", ms );
        OutputDebugStringA( asd );
        platform::sleep( std::max( 1, ms ) );*/
        platform::sleep( 2 );
      }
    }
  }

  void Engine::shutdown()
  {
    scripting_.reset();

    if ( renderer_ )
      renderer_->stop();

    if ( loader_ )
      loader_->stop();

    loader_.reset();

    if ( fonts_ )
      fonts_->shutdown();

    renderer_.reset();

    fonts_.reset();

    messaging_.reset();
    Locator::provideMessaging( MessagingPtr() );

    tanklib_.unload();

    console_->resetEngine();
  }

}