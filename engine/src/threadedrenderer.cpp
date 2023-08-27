#include "pch.h"
#include "neko_exception.h"
#include "engine.h"
#include "gfx.h"
#include "shaders.h"
#include "renderer.h"
#include "camera.h"
#include "font.h"
#include "console.h"
#include "messaging.h"
#include "gui.h"
#include "neko_types.h"

namespace neko {

  const string c_gfxThreadName = "nekoRenderer";

  ThreadedRenderer::ThreadedRenderer( EnginePtr engine, ThreadedLoaderPtr loader, FontManagerPtr fonts,
    MessagingPtr messaging, DirectorPtr director, ConsolePtr console ):
    engine_( move( engine ) ),
    loader_( move( loader ) ), fonts_( move( fonts ) ), messaging_( move( messaging ) ), director_( move( director ) ),
    console_( move( console ) ), thread_( c_gfxThreadName, threadProc, this )
  {
  }

  // Context: Renderer thread
  void ThreadedRenderer::initialize()
  {
    gfx_ = make_shared<Gfx>( loader_, fonts_, messaging_, director_, console_ );
    gfx_->postInitialize( *engine_.get() );
    lastTime_ = 0.0;
    lastRealTime_ = 0.0;
  }

  // Context: Renderer thread
  void ThreadedRenderer::shutdown()
  {
    gfx_.reset();
  }

  // Context: Renderer thread
  void ThreadedRenderer::run( platform::Event& wantStop )
  {
    while ( true )
    {
      if ( wantStop.check() )
        break;

      if ( restartEvent_.check() )
      {
        restartEvent_.reset();
        console_->printf( srcGfx, "Restarting renderer" );
        gfx_->logicLock_.lock();
        gfx_->jsRestart( *engine_ );
        gfx_->logicLock_.unlock();
        console_->printf( srcGfx, "Restart done" );
        restartedEvent_.set();
        continue;
      }

      gfx_->logicLock_.lock();

      gfx_->preUpdate();

      auto discardMouse = ImGui::GetIO().WantCaptureMouse;
      auto discardKeyboard = ImGui::GetIO().WantCaptureKeyboard;

      gfx_->processEvents( *engine_, discardMouse, discardKeyboard );

      auto time = engine_->sync().gameTime.load();
      auto delta = ( time - lastTime_ );
      lastTime_ = time;

      gfx_->update( time, delta, *engine_ );

      auto rt = engine_->sync().realTime.load();
      delta = ( rt - lastRealTime_ );
      lastRealTime_ = rt;

      gfx_->updateRealTime( rt, delta, *engine_ );

      gfx_->logicLock_.unlock();

      platform::sleep( 1 );
    }

    gfx_->shutdown( *engine_ );
  }

  bool ThreadedRenderer::threadProc( platform::Event& running, platform::Event& wantStop, void* argument )
  {
    platform::performanceInitializeRenderThread();

    auto myself = ( (ThreadedRenderer*)argument )->shared_from_this();
    myself->initialize();
    running.set();
    myself->run( wantStop );
    myself->shutdown();

    platform::performanceTeardownCurrentThread();
    return true;
  }

  void ThreadedRenderer::start()
  {
    thread_.start();
  }

  void ThreadedRenderer::restart()
  {
    restartedEvent_.reset();
    restartEvent_.set();
    restartedEvent_.wait();
  }

  void ThreadedRenderer::stop()
  {
    thread_.stop();
  }

  ThreadedRenderer::~ThreadedRenderer()
  {
    //
  }

}