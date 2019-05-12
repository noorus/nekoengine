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
    scripting_ = make_shared<Scripting>( shared_from_this() );
    scripting_->initialize();
    console_->printf( Console::srcScripting, "Scripting init took %dms", (int)timer.stop() );
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
      gfx_->preUpdate( time_ );

      delta = clock_.update();
      accumulator += delta;

      while ( accumulator >= cLogicStep )
      {
        gfx_->tick( cLogicStep, time_ );
        time_ += cLogicStep;
        accumulator -= cLogicStep;
      }

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

    gfx_.reset();

    loader_.reset();

    console_->resetEngine();
  }

}