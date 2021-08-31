#include "stdafx.h"
#include "neko_exception.h"
#include "engine.h"
#include "gfx.h"
#include "shaders.h"
#include "renderer.h"
#include "camera.h"
#include "fontmanager.h"
#include "console.h"
#include "messaging.h"
#include "gui.h"

#pragma comment( lib, "opengl32.lib" )

#ifdef _DEBUG
# pragma comment( lib, "glbindingd.lib" )
# pragma comment( lib, "glbinding-auxd.lib" )
# pragma comment( lib, "sfml-main-d.lib" )
# pragma comment( lib, "sfml-system-d.lib" )
# pragma comment( lib, "sfml-window-d.lib" )
# ifndef NEKO_NO_GUI
#  pragma comment( lib, "MyGUIEngine_d.lib" )
# endif
# ifndef NEKO_NO_ANIMATION
#  pragma comment( lib, "ozz_animation_d.lib" )
#  pragma comment( lib, "ozz_animation_fbx_d.lib" )
#  pragma comment( lib, "ozz_animation_tools_d.lib" )
#  pragma comment( lib, "ozz_animation_offline_d.lib" )
#  pragma comment( lib, "ozz_base_d.lib" )
#  pragma comment( lib, "ozz_geometry_d.lib" )
#  pragma comment( lib, "ozz_options_d.lib" )
# endif
#else
# pragma comment( lib, "glbinding.lib" )
# pragma comment( lib, "sfml-main.lib" )
# pragma comment( lib, "sfml-system.lib" )
# pragma comment( lib, "sfml-window.lib" )
# ifndef NEKO_NO_GUI
#  pragma comment( lib, "MyGUIEngine.lib" )
# endif
# ifndef NEKO_NO_ANIMATION
#  pragma comment( lib, "ozz_animation_r.lib" )
#  pragma comment( lib, "ozz_animation_fbx_r.lib" )
#  pragma comment( lib, "ozz_animation_tools_r.lib" )
#  pragma comment( lib, "ozz_animation_offline_r.lib" )
#  pragma comment( lib, "ozz_base_r.lib" )
#  pragma comment( lib, "ozz_geometry_r.lib" )
#  pragma comment( lib, "ozz_options_r.lib" )
# endif
#endif

namespace neko {

  using namespace gl;

  // Gfx

#ifdef _DEBUG
  const char c_windowTitle[] = "Panzer Pandemonium [debug]";
#else
  const char c_windowTitle[] = "Panzer Pandemonium [release]";
#endif

  NEKO_DECLARE_CONVAR( vid_screenwidth, "Screen width. Changes are applied when the renderer is restarted.", 1920 );
  NEKO_DECLARE_CONVAR( vid_screenheight, "Screen height. Changes are applied when the renderer is restarted.", 1080 );
  NEKO_DECLARE_CONVAR( vid_vsync, "Whether to print OpenGL debug log output.", true );
  NEKO_DECLARE_CONVAR( gl_debuglog, "OpenGL debug log output level. 0 = none, 1 = some, 2 = debug context", 2 );

  Gfx::Gfx( ThreadedLoaderPtr loader, FontManagerPtr fonts, MessagingPtr messaging, DirectorPtr director, ConsolePtr console ):
  loader_( move( loader ) ), fonts_( move( fonts ) ), console_( move( console ) ),
  messaging_( move( messaging ) ), director_( move( director ) )
  {
    assert( loader_ && fonts_ && director_ && messaging_ && console_ );
    preInitialize();
    input_ = make_shared<GfxInput>( console_ );
    gui_ = make_shared<GUI>();
  }

  void Gfx::printInfo()
  {
    console_->print( Console::srcGfx, "GL Vendor: " + info_.vendor_ );
    console_->print( Console::srcGfx, "GL Version: " + info_.version_ );
    console_->print( Console::srcGfx, "GL Renderer: " + info_.renderer_ );
  }

  // 131185: Buffer detailed info...
  // -> Information on buffer object creation, hinting and memory placement
  // 131218: Program/shader state performance warning...
  // 131204: The texture object does not have a defined base level...
  // 131154: Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
  // -> Basically just means that we're not using NV-specific (?) multithreaded texture uploads
  // -> https://on-demand.gputechconf.com/gtc/2012/presentations/S0356-GTC2012-Texture-Transfers.pdf
  const std::array<GLuint, 4> c_ignoredGlDebugMessages = { 131185, 131218, 131204, 131154 };

  void Gfx::openglDebugCallbackFunction( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam )
  {
    for ( GLuint ignoredId : c_ignoredGlDebugMessages )
      if ( id == ignoredId )
        return;

    auto gfx = static_cast<Gfx*>( const_cast<void*>( userParam ) );

    if ( id == 0
      || id == 1281
      || id == 1282
      || id == 1285
      // || id == 131076
      )
      DebugBreak();

    if ( !gfx )
      return;

    if ( ( (uintptr_t)gfx & 0xCCCCCCCC ) == 0xCCCCCCCC || severity == GL_DEBUG_TYPE_ERROR )
    {
      utf8String msg = "OpenGL Error: ";
      msg.append( message );
      msg.append( "\r\n" );
      OutputDebugStringA( msg.c_str() );
      NEKO_EXCEPT( "OpenGL Error" );
    }
    else
      gfx->console_->printf( Console::srcGfx, "OpenGL Debug: %s (%d)", message, id );
  }

  void Gfx::setOpenGLDebugLogging( const bool enable )
  {
    if ( enable )
    {
      glEnable( GL_DEBUG_OUTPUT );
      glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
      glDebugMessageCallback( Gfx::openglDebugCallbackFunction, this );
      glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
    }
    else
    {
      glDisable( GL_DEBUG_OUTPUT );
      glDisable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
    }
  }

  void Gfx::preInitialize()
  {
    info_.clear();

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::VideoMode videoMode( g_CVar_vid_screenwidth.as_i(), g_CVar_vid_screenheight.as_i(), desktop.bitsPerPixel );

    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 0;
    settings.majorVersion = c_glVersion[0];
    settings.minorVersion = c_glVersion[1];
    settings.attributeFlags = sf::ContextSettings::Attribute::Core;

    if ( g_CVar_gl_debuglog.as_i() > 1 )
      settings.attributeFlags |= sf::ContextSettings::Attribute::Debug;

    window_ = make_unique<sf::Window>( videoMode, c_windowTitle, sf::Style::Default, settings );

    platform::setWindowIcon( window_->getSystemHandle(), platform::KnownIcon::MainIcon );

    window_->setVerticalSyncEnabled( g_CVar_vid_vsync.as_b() ); // vsync
    window_->setFramerateLimit( 0 ); // no sleep till Brooklyn

    window_->setActive( true );

    const auto targetResolution = size2i( 1920, 1080 );

    platform::RenderWindowHandler::get().setWindow( this, window_->getSystemHandle() );
    platform::RenderWindowHandler::get().changeTargetResolution( targetResolution );

    glbinding::initialize( nullptr );

    auto glstrGetClean = []( GLenum e, string& out )
    {
      auto str = glGetString( e );
      if ( str )
        out = (const char*)str;
    };

    glstrGetClean( GL_RENDERER, info_.renderer_ );
    glstrGetClean( GL_VERSION, info_.version_ );
    glstrGetClean( GL_VENDOR, info_.vendor_ );

    printInfo();
  }

  void Gfx::postInitialize( Engine& engine )
  {
    renderer_ = make_shared<Renderer>( loader_, fonts_, director_, console_ );
    renderer_->preInitialize();
    target_ = renderer_->createSceneNode( nullptr );

    setOpenGLDebugLogging( g_CVar_gl_debuglog.as_i() > 0 );

    auto realResolution = vec2( (Real)window_->getSize().x, (Real)window_->getSize().y );
    camera_ = make_unique<ArcballCamera>( renderer_.get(), realResolution, target_,
      vec3( 5.0f, 3.0f, 5.0f ), // vecOffset
      60.0f, // fov
      true, // reverse
      10.0f, // sens
      5.0f, // mindist
      50.0f, // maxdist
      2.0f, // rotdecel
      5.0f, // zoomaccel
      2.0f ); // zoomdecel

    resize( window_->getSize().x, window_->getSize().y );

    renderer_->initialize( viewport_.size_.x, viewport_.size_.y );

    auto documentsPath = platform::wideToUtf8( engine.env().documentsPath_ );
    gui_->initialize( this, documentsPath, *window_.get() );
    gui_->poop();

    input_->initialize( window_->getSystemHandle() );

    messaging_->listen( this );
  }

  void Gfx::resize( size_t width, size_t height )
  {
    viewport_.size_ = vec2i( width, height );
    input_->setWindowSize( viewport_.size_ );

    console_->printf( Console::srcGfx, "Setting viewport to %dx%d", width, height );

    auto realResolution = vec2( (Real)width, (Real)height );
    camera_->setViewport( realResolution );

    glViewport( 0, 0, (GLsizei)width, (GLsizei)height );

    gui_->resize( static_cast<int>( width ), static_cast<int>( height ) );
  }

  void Gfx::processEvents()
  {
    input_->update();

    sf::Event evt;
    while ( window_->pollEvent( evt ) )
    {
      if ( evt.type == sf::Event::Closed )
      {
        messaging_->send( M_Window_Close );
      }
      else if ( evt.type == sf::Event::Resized )
      {
        resize( evt.size.width, evt.size.height );
        flags_.resized = true;
      }
      else if ( evt.type == sf::Event::LostFocus )
      {
        messaging_->send( M_Window_LostFocus );
      }
      else if ( evt.type == sf::Event::GainedFocus )
      {
        messaging_->send( M_Window_GainedFocus );
      }
      else if ( evt.type == sf::Event::KeyPressed )
      {
        if ( evt.key.code == sf::Keyboard::F5 )
          messaging_->send( M_Debug_ReloadScript );
        if ( evt.key.code == sf::Keyboard::F6 )
        {
          messaging_->send( M_Debug_ReloadShaders );
          flags_.reloadShaders = true;
        }
      }
    }
  }

  void Gfx::onMessage( const Message& msg )
  {
    logicLock_.lock();
    logicLock_.unlock();
  }

  void Gfx::update( GameTime time, GameTime delta )
  {
    if ( flags_.reloadShaders )
    {
      renderer_->shaders().shutdown();
      renderer_->shaders().initialize();
      flags_.reloadShaders = false;
    }

    renderer_->prepare( time );

    char asd[256];
    sprintf_s( asd, 256, "[%s,%s,%s,%s,%s]\nmouse x %lli y %lli z %lli",
      input_->mouseButtons_[0] ? "true" : "false",
      input_->mouseButtons_[1] ? "true" : "false",
      input_->mouseButtons_[2] ? "true" : "false",
      input_->mouseButtons_[3] ? "true" : "false",
      input_->mouseButtons_[4] ? "true" : "false",
      input_->movement_.x, input_->movement_.y, input_->movement_.z );
    gui_->setshit( asd );

    if ( input_->mousebtn( 2 ) )
      camera_->applyInputPanning( input_->movement() );
    else if ( input_->mousebtn( 1 ) )
      camera_->applyInputRotation( input_->movement() );

    camera_->applyInputZoom( static_cast<int>( input_->movement().z ) );

    camera_->update( delta, time );

    // activate as current context
    window_->setActive( true );

    if ( flags_.resized )
    {
      renderer_->reset( viewport_.size_.x, viewport_.size_.y );
      flags_.resized = false;
      window_->setActive( true );
    }

#ifndef NEKO_NO_GUI
    renderer_->draw( time, *camera_.get(), gui_->platform() );
#else
    renderer_->draw( time, *camera_.get(), nullptr );
#endif

    window_->display();
  }

  const Image& Gfx::renderWindowReadPixels()
  {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    lastCapture_.size_ = size2i( viewport_.size_ );
    lastCapture_.buffer_.resize( lastCapture_.size_.w * lastCapture_.size_.h * 4 * sizeof( uint8_t ) );
    glReadnPixels( 0, 0,
      (GLsizei)lastCapture_.size_.w, (GLsizei)lastCapture_.size_.h,
      GL_BGRA, GL_UNSIGNED_BYTE,
      (GLsizei)lastCapture_.buffer_.size(), lastCapture_.buffer_.data() );
    return lastCapture_;
  }

  void Gfx::shutdown()
  {
    messaging_->remove( this );

    if ( target_ )
    {
      renderer_->destroySceneNode( target_ );
      target_ = nullptr;
    }

    input_->shutdown();

    gui_->shutdown();

    camera_.reset();

    platform::RenderWindowHandler::get().setWindow( nullptr, nullptr );

    window_->close();
    window_.reset();

    platform::RenderWindowHandler::free();
  }

  void Gfx::jsRestart()
  {
    renderer_->jsRestart();
    renderer_->prepare( 0.0 );
  }

  void Gfx::restart( Engine& engine )
  {
    shutdown();
    preInitialize();
    postInitialize( engine );
  }

  Gfx::~Gfx()
  {
    shutdown();
    input_.reset();
  }

  const string c_gfxThreadName = "nekoRenderer";

  ThreadedRenderer::ThreadedRenderer( EnginePtr engine, ThreadedLoaderPtr loader,
  FontManagerPtr fonts, MessagingPtr messaging, DirectorPtr director, ConsolePtr console ):
  engine_( move( engine ) ), loader_( move( loader ) ), fonts_( move( fonts ) ),
  messaging_( move( messaging ) ), director_( move( director ) ), lastTime_( 0.0 ),
  console_( move( console ) ), thread_( c_gfxThreadName, threadProc, this )
  {
  }

  void ThreadedRenderer::initialize()
  {
    gfx_ = make_shared<Gfx>( loader_, fonts_, messaging_, director_, console_ );
    gfx_->postInitialize( *engine_.get() );
    lastTime_ = 0.0;
  }

  void ThreadedRenderer::run( platform::Event& wantStop )
  {
    while ( true )
    {
      if ( wantStop.check() )
        break;

      if ( restartEvent_.check() )
      {
        restartEvent_.reset();
        console_->printf( Console::srcGfx, "Restarting renderer" );
        gfx_->logicLock_.lock();
        gfx_->jsRestart();
        gfx_->logicLock_.unlock();
        console_->printf( Console::srcGfx, "Restart done" );
        restartedEvent_.set();
        continue;
      }

      gfx_->logicLock_.lock();

      gfx_->processEvents();

      auto time = engine_->renderTime().load();
      auto delta = ( time - lastTime_ );
      lastTime_ = time;

      gfx_->update( time, delta );

      gfx_->logicLock_.unlock();

      platform::sleep( 1 );
    }
  }

  bool ThreadedRenderer::threadProc( platform::Event& running, platform::Event& wantStop, void* argument )
  {
    platform::performanceInitializeRenderThread();

    auto myself = ( (ThreadedRenderer*)argument )->shared_from_this();
    myself->initialize();
    running.set();
    myself->run( wantStop );

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