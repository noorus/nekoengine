#pragma once
#include "stdafx.h"
#include "engine.h"
#include "console.h"
#include "neko_exception.h"
#include "gfx.h"

namespace neko {

  GameTime fTime = 0.0;
  GameTime fTimeDelta = 0.0;
  GameTime fTimeAccumulator = 0.0;
  GameTime fLogicStep = 1.0 / 60.0;
  LARGE_INTEGER hpcFrequency;

  Engine::Engine( ConsolePtr console ): console_( move( console ) )
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

    // Fetch HPC frequency
    if ( !QueryPerformanceFrequency( &hpcFrequency ) )
      NEKO_EXCEPT( "Couldn't query HPC frequency" );

    gfx_ = make_shared<Gfx>( shared_from_this() );
    gfx_->postInitialize();
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
    LARGE_INTEGER timeCurrent;
    LARGE_INTEGER tickDelta;
    LARGE_INTEGER timeNew;

    QueryPerformanceCounter( &timeCurrent );

    fTime = 0.0;
    fTimeAccumulator = 0.0;

    while ( signal_ != Signal_Stop )
    {
      gfx_->preUpdate( fTime );

      QueryPerformanceCounter( &timeNew );

      tickDelta.QuadPart = timeNew.QuadPart - timeCurrent.QuadPart;
      timeCurrent = timeNew;

      fTimeDelta = (GameTime)tickDelta.QuadPart / (GameTime)hpcFrequency.QuadPart;
      fTimeAccumulator += fTimeDelta;

      while ( fTimeAccumulator >= fLogicStep )
      {
        gfx_->tick( fLogicStep, fTime );
        fTime += fLogicStep;
        fTimeAccumulator -= fLogicStep;
      }

      if ( fTimeDelta > 0.0 && signal_ != Signal_Stop )
      {
        gfx_->postUpdate( fTimeDelta, fTime );
      }
    }
  }

  void Engine::shutdown()
  {
    gfx_.reset();

    console_->resetEngine();
  }

}