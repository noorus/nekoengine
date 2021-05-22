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

  GameTime cLogicStep = ( 1.0 / 60.0 ); //!< 60 fps

  Engine::Engine( ConsolePtr console ): console_( move( console ) ), time_( 0.0 )
  {
  }

  Engine::~Engine()
  {
    shutdown();
  }

  void Engine::initialize( const Options& options )
  {
    console_->setEngine( shared_from_this() );

    platform::PerformanceTimer timer;

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
    console_->printf( Console::srcEngine, "Font manager init took %dms", (int)timer.stop() );

    timer.start();
    renderer_ = make_shared<ThreadedRenderer>( loader_, fonts_, messaging_, director_, console_ );
    renderer_->start();
    console_->printf( Console::srcGfx, "Renderer init took %dms", (int)timer.stop() );

#ifndef NEKO_NO_SCRIPTING
    timer.start();
    scripting_ = make_shared<Scripting>( shared_from_this() );
    scripting_->initialize();
    console_->printf( Console::srcScripting, "Scripting init took %dms", (int)timer.stop() );
#endif

    scripting_->postInitialize();
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
    }
  }

  bool Engine::paused()
  {
    if ( state_.focusLost || state_.windowMove )
      return true;
    return false;
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

      delta = clock_.update();

      //gfx_->processEvents();
      messaging_->processEvents();

      scripting_->preUpdate( time_ );
      fonts_->prepare( time_ );
      //gfx_->preUpdate( time_ );

      if ( !paused() )
      {
        accumulator += delta;
        while ( accumulator >= cLogicStep )
        {
          scripting_->tick( cLogicStep, time_ );
          messaging_->tick( cLogicStep, time_ );
          //gfx_->tick( cLogicStep, time_ );
          time_ += cLogicStep;
          accumulator -= cLogicStep;
        }
      }

      scripting_->postUpdate( delta, time_ );

      Sleep( 10 );

      if ( delta > 0.0 && signal_ != Signal_Stop )
      {
        //gfx_->postUpdate( delta, time_ );
      }
    }
  }

  void Engine::shutdown()
  {
    if ( renderer_ )
      renderer_->stop();

    if ( loader_ )
      loader_->stop();

    loader_.reset();

    if ( fonts_ )
      fonts_->shutdown();

    renderer_.reset();

    fonts_.reset();

    scripting_.reset();

    messaging_.reset();
    Locator::provideMessaging( MessagingPtr() );

    console_->resetEngine();
  }

}