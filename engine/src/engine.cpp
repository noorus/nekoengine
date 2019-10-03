#pragma once
#include "stdafx.h"
#include "engine.h"
#include "console.h"
#include "neko_exception.h"
#include "gfx.h"
#include "scripting.h"
#include "loader.h"

namespace neko {

  GameTime cLogicStep = 1.0 / 60.0; //!< 60 fps

  Engine::Engine( ConsolePtr console ): console_( move( console ) ), time_( 0.0 )
  {
  }

  Engine::~Engine()
  {
    shutdown();
  }

  void Engine::operationSuspendVideo()
  {
  }

  void Engine::operationContinueVideo()
  {
  }

  void Engine::operationSuspendAudio()
  {
  }

  void Engine::operationContinueAudio()
  {
  }

  void Engine::initialize( const Options& options )
  {
    console_->setEngine( shared_from_this() );

    platform::PerformanceTimer timer;

    loader_ = make_shared<ThreadedLoader>();
    loader_->start();

    timer.start();
    gfx_ = make_shared<Gfx>( shared_from_this() );
    gfx_->postInitialize();
    console_->printf( Console::srcGfx, "Gfx init took %dms", (int)timer.stop() );

    timer.start();
    fonts_ = make_shared<FontManager>();
    console_->printf( Console::srcEngine, "Font manager init took %dms", (int)timer.stop() );

#ifndef NEKO_NO_SCRIPTING
    timer.start();
    scripting_ = make_shared<Scripting>( shared_from_this() );
    scripting_->initialize();
    console_->printf( Console::srcScripting, "Scripting init took %dms", (int)timer.stop() );
#endif

    scripting_->postInitialize();
  }

  void Engine::signalStop()
  {
    signal_ = Signal_Stop;
  }

  void Engine::triggerFatalError( FatalError error )
  {
    signalStop();
  }

  void Engine::run()
  {
    clock_.init();
    time_ = 0.0;

    GameTime accumulator = 0.0;
    GameTime delta = 0.0;

    while ( signal_ != Signal_Stop )
    {
      console_->executeBuffered();
      scripting_->preUpdate( time_ );
      gfx_->preUpdate( time_ );

      delta = clock_.update();
      accumulator += delta;

      while ( accumulator >= cLogicStep )
      {
        scripting_->tick( cLogicStep, time_ );
        gfx_->tick( cLogicStep, time_ );
        time_ += cLogicStep;
        accumulator -= cLogicStep;
      }

      scripting_->postUpdate( delta, time_ );

      if ( delta > 0.0 && signal_ != Signal_Stop )
      {
        gfx_->postUpdate( delta, time_ );
      }
    }
  }

  void Engine::shutdown()
  {
    scripting_.reset();

    if ( loader_ )
      loader_->stop();

    loader_.reset();

    gfx_.reset();

    fonts_.reset();

    console_->resetEngine();
  }

}