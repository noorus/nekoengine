#pragma once
#include "pch.h"
#include "engine.h"
#include "console.h"
#include "neko_exception.h"
#include "gfx.h"
#include "scripting.h"
#include "loader.h"
#include "messaging.h"
#include "input.h"
#include "director.h"

namespace neko {

  GameTime c_logicFPS = 60.0; //!< 60 fps
  GameTime c_logicStep = ( 1.0 / c_logicFPS ); //!< tick ms
  uint64_t c_logicMaxFrameMicroseconds = static_cast<uint64_t>( ( c_logicStep * 1000.0 ) * 1000.0 ); //!< max us

  const char* c_engineName = "Nekoengine Alpha";
  const char* c_engineLogName = "nekoengine";
  const uint32_t c_engineVersion[3] = { 0, 1, 3 };

#ifdef _DEBUG
  const wchar_t* c_rainetLibraryName = L"rainet_d.dll";
#else
  const wchar_t* c_rainetLibraryName = L"rainet.dll";
#endif

  const char* c_engineSettingsFilename = "engineconf.json";

  Engine::Engine( ConsolePtr console, const Environment& env ):
  console_( move( console ) ), env_( env )
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

  const utf8String& Engine::listFlags()
  {
    static const utf8String flags = ""
#ifdef NEKO_NO_AUDIO
      "noaudio "
#endif
#ifdef NEKO_NO_GUI
      "nogui "
#endif
#ifdef NEKO_NO_SCRIPTING
      "noscripting "
#endif
#ifdef NEKO_NO_RAINET
      "norainet "
#endif
      "windows";
    return flags;
  }

#ifndef NEKO_NO_RAINET
  void RainetLibrary::load( rainet::Host* host )
  {
    module_ = LoadLibraryW( c_rainetLibraryName );
    if ( !module_ )
      NEKO_EXCEPT( "Rainet load failed" );
    pfnInitialize = reinterpret_cast<rainet::fnInitialize>( GetProcAddress( module_, "rainetInitialize" ) );
    pfnShutdown = reinterpret_cast<rainet::fnShutdown>( GetProcAddress( module_, "rainetShutdown" ) );
    if ( !pfnInitialize || !pfnShutdown )
      NEKO_EXCEPT( "Rainet export resolution failed" );
    engine_ = pfnInitialize( rainet::c_headerVersion, host );
    if ( !engine_ )
      NEKO_EXCEPT( "Rainet init failed" );
  }

  void RainetLibrary::unload()
  {
    if ( engine_ && pfnShutdown )
    {
      pfnShutdown( engine_ );
      engine_ = nullptr;
    }
    if ( module_ )
      FreeLibrary( module_ );
  }
#endif

  void parseSettingsJSON( const json& settings, EngineSettings& out )
  {
    if ( !settings.is_object() )
      return;
    if ( settings.find( "steam" ) != settings.end() )
    {
      auto& steam = settings["steam"];
      out.steamAppID = utils::parseUint32( steam["appid"].get<utf8String>().c_str() );
      if ( out.steamAppID )
        out.useSteam = true;
    }
    if ( settings.find( "discord" ) != settings.end() )
    {
      auto& discord = settings["discord"];
      out.discordAppID = utils::parseUint64( discord["appid"].get<utf8String>().c_str() );
      if ( out.discordAppID )
        out.useDiscord = true;
    }
  }

  void Engine::initialize( const Options& options )
  {
    console_->setEngine( shared_from_this() );

    console_->printf( Console::srcEngine, "Build flags: %s", listFlags().c_str() );

    auto settingstext = Locator::fileSystem().openFile( Dir_User, c_engineSettingsFilename )->readFullString();
    parseSettingsJSON( json::parse( settingstext ), settings_ );

    platform::PerformanceTimer timer;

    UVersionInfo icuVersion;
    u_getVersion( icuVersion );
    console_->printf( Console::srcEngine, "Using ICU v%d.%d.%d", icuVersion[0], icuVersion[1], icuVersion[2] );

#ifndef NEKO_NO_RAINET
    rainet_.load( this );
    rainet_.engine_->initialize( settings_.discordAppID, settings_.steamAppID );
#endif

    loader_ = make_shared<ThreadedLoader>();
    loader_->start();

    messaging_ = make_shared<Messaging>( shared_from_this() );
    Locator::provideMessaging( messaging_ ); // WARN Technically a bad pattern, but fine as long as there's one Engine
    messaging_->listen( this );

    director_ = make_shared<Director>();
    director_->reset();
    Locator::provideDirector( director_ );

    timer.start();
    fonts_ = make_shared<FontManager>( loader_ );
    fonts_->initializeLogic();
    console_->printf( Console::srcGfx, "Font manager init took %dms", (int)timer.stop() );

    timer.start();
    renderer_ = make_shared<ThreadedRenderer>( shared_from_this(), loader_, fonts_, messaging_, director_, console_ );
    renderer_->start();
    console_->printf( Console::srcGfx, "Renderer init took %dms", (int)timer.stop() );

#ifndef NEKO_NO_SCRIPTING
    timer.start();
    scripting_ = make_shared<Scripting>( shared_from_this() );
    scripting_->initialize();
    console_->printf( Console::srcScripting, "Scripting init took %dms", (int)timer.stop() );
#endif

#ifndef NEKO_NO_RAINET
    rainet_.engine_->update( 0.0, 0.0 );
#endif

#ifndef NEKO_NO_SCRIPTING
    scripting_->postInitialize();
#endif
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
      case M_Debug_ToggleDevMode:
        state_.devMode = !state_.devMode;
        break;
      case M_Debug_PauseTime:
        state_.timePaused = !state_.timePaused;
        console_->printf( Console::srcEngine, "Time %s", state_.timePaused ? "paused" : "unpaused" );
        break;
    }
  }

#ifndef NEKO_NO_RAINET

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
    if ( enabled != state_.steamOverlay )
    {
      messaging_->send( M_Extern_SteamOverlay, 1, enabled );
      state_.steamOverlay = enabled;
    }
  }

  void Engine::onSteamStatsUpdated( rainet::StateUpdateIndex index )
  {
    if ( index == 0 )
      rainet_.engine_->statIncrement( "dev_launches" );
    stats_.i_launches.store( rainet_.engine_->steamStats().at( "dev_launches" ).i_ );
    stats_.f_timeWasted.store( rainet_.engine_->steamStats().at( "dev_debugTime" ).f_ );
  }

  void Engine::onAccountUpdated( const rainet::Account& user )
  {
    if ( user.steamId_ && user.steamImage_ )
      messaging_->send( M_Extern_AccountUpdated, 1, static_cast<size_t>( user.id_ ) );
  }

#endif

  bool Engine::paused()
  {
    if ( state_.focusLost || state_.windowMove || state_.steamOverlay || state_.timePaused )
      return true;
    return false;
  }

  void Engine::restart()
  {
#ifndef NEKO_NO_SCRIPTING
    scripting_->shutdown();
#endif
    renderer_->restart();
#ifndef NEKO_NO_SCRIPTING
    scripting_->initialize();
    scripting_->postInitialize();
#endif
  }

  const rainet::GameInstallationState& Engine::installationInfo()
  {
#ifndef NEKO_NO_RAINET
    if ( rainet_.engine_->startedFromSteam() )
      return rainet_.engine_->steamInstallation();
    return rainet_.engine_->localInstallation();
#else
    static rainet::GameInstallationState fuckyou = { .installPath_ = platform::wideToUtf8( platform::getCurrentDirectory() ) };
    return fuckyou;
#endif
  }

  void Engine::run()
  {
    clock_.init();
    time_ = 0.0;
    realTime_ = 0.0;

    GameTime accumulator = 0.0;
    GameTime delta = 0.0;

    const auto maxSleepytimeMs = math::ifloor( (double)c_logicMaxFrameMicroseconds / 1000.0 );

    console_->printf( Console::srcEngine, "Logic: targeting %.02f FPS, logic step %.02fms, max frame time %I64uus, max sleep %ims",
      (float)c_logicFPS, static_cast<float>( c_logicStep * 1000.0 ), c_logicMaxFrameMicroseconds, maxSleepytimeMs );

    // this thing is just for silly stats
    platform::PerformanceTimer overallTime;
    overallTime.start();

#ifndef NEKO_NO_RAINET
    rainet_.engine_->update( time_, 0.0 );
    rainet_.engine_->changeActivity_AlphaDevelop();
#endif

    while ( signal_ != Signal_Stop )
    {
      delta = clock_.update();

      console_->executeBuffered();

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

#ifndef NEKO_NO_SCRIPTING
      scripting_->preUpdate( time_ );
#endif
      fonts_->prepareLogic( time_ );

      if ( !paused() )
      {
        accumulator += delta;
        while ( accumulator >= c_logicStep )
        {
#ifndef NEKO_NO_SCRIPTING
          scripting_->tick( c_logicStep, time_ );
#endif
          messaging_->tick( c_logicStep, time_ );
          time_ += c_logicStep;
          accumulator -= c_logicStep;
        }
      }

      realTime_ += delta;

      sync_.gameTime.store( time_ );
      sync_.realTime.store( realTime_ );

#ifndef NEKO_NO_RAINET
      rainet_.engine_->update( time_, delta );
#endif

#ifndef NEKO_NO_SCRIPTING
      scripting_->postUpdate( delta, time_ );
#endif

      auto us = clock_.peekMicroseconds();
      if ( us >= c_logicMaxFrameMicroseconds )
      {
        console_->printf( Console::srcEngine, "WARNING: Logic frame exceeded max time (%I64uus)", us );
      }
      else
      {
        /* this pretty sleep math is for if you really want to save on CPU, but makes for some jitter.
        auto ms = math::ifloor( static_cast<double>( cLogicMaxFrameMicroseconds - us ) / 1000.0 );
        platform::sleep( std::max( 1, ms ) );*/
        platform::sleep( 2 );
      }
    }

#ifndef NEKO_NO_RAINET
    auto secondsDebugged = static_cast<float>( overallTime.stop() / 1000.0 );
    rainet_.engine_->statAdd( "dev_debugTime", secondsDebugged );
    rainet_.engine_->uploadStats();
#endif
  }

  void Engine::shutdown()
  {
#ifndef NEKO_NO_SCRIPTING
    scripting_.reset();
#endif

    if ( renderer_ )
      renderer_->stop();

    if ( loader_ )
      loader_->stop();

    loader_.reset();

    if ( fonts_ )
      fonts_->shutdownLogic();

    renderer_.reset();

    fonts_.reset();

    messaging_.reset();
    Locator::provideMessaging( MessagingPtr() );

#ifndef NEKO_NO_RAINET
    rainet_.unload();
#endif

    console_->resetEngine();
  }

}