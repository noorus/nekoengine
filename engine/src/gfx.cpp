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

namespace neko {

  using namespace gl;

  // Gfx

  const char cWindowTitle[] = "nekoengine//render";

  NEKO_DECLARE_CONVAR( vid_screenwidth,
    "Screen width. Changes are applied when the renderer is restarted.", 1280 );
  NEKO_DECLARE_CONVAR( vid_screenheight,
    "Screen height. Changes are applied when the renderer is restarted.", 720 );
  NEKO_DECLARE_CONVAR( gl_debuglog,
    "Whether to print OpenGL debug log output.", true );

  Gfx::Gfx( ThreadedLoaderPtr loader, FontManagerPtr fonts, MessagingPtr messaging, DirectorPtr director, ConsolePtr console )
      : loader_( move( loader ) ), fonts_( move( fonts ) ), console_( move( console ) ), messaging_( move( messaging ) ),
    director_( move( director ) )
  {
    assert( loader_ && fonts_ && director_  && messaging_ && console_ );

    preInitialize();
  }

  void Gfx::printInfo()
  {
    console_->print( Console::srcGfx, "GL Vendor: " + info_.vendor_ );
    console_->print( Console::srcGfx, "GL Version: " + info_.version_ );
    console_->print( Console::srcGfx, "GL Renderer: " + info_.renderer_ );
  }

  // 131185: Buffer detailed info...
  // 131218: Program/shader state performance warning...
  // 131204: The texture object does not have a defined base level...
  // 131154: Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
  const std::array<GLuint, 4> c_ignoredGlDebugMessages = { 131185, 131218, 131204, 131154 };

  void Gfx::openglDebugCallbackFunction( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam )
  {
    for ( GLuint ignoredId : c_ignoredGlDebugMessages )
      if ( id == ignoredId )
        return;

    auto gfx = static_cast<Gfx*>( const_cast<void*>( userParam ) );

    if ( id == 1281 || id == 1285 || id == 0 )
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
    settings.majorVersion = 4;
    settings.minorVersion = 6;
    settings.attributeFlags = sf::ContextSettings::Attribute::Core | sf::ContextSettings::Attribute::Debug;

    window_ = make_unique<sf::Window>( videoMode, cWindowTitle, sf::Style::Default, settings );

    window_->setVerticalSyncEnabled( true ); // vsync
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

  void Gfx::postInitialize()
  {
    renderer_ = make_shared<Renderer>( loader_, fonts_, director_, console_ );
    renderer_->preInitialize();

    setOpenGLDebugLogging( g_CVar_gl_debuglog.as_b() );

    auto realResolution = vec2( (Real)window_->getSize().x, (Real)window_->getSize().y );
    camera_ = make_unique<Camera>( realResolution, vec3( 0.0f, 0.0f, 0.0f ) );

    resize( window_->getSize().x, window_->getSize().y );

    renderer_->initialize( viewport_.size_.x, viewport_.size_.y );
  }

  void Gfx::preUpdate( GameTime time )
  {
    renderer_->prepare( time );
  }

  void Gfx::resize( size_t width, size_t height )
  {
    viewport_.size_ = vec2i( width, height );

    console_->printf( Console::srcGfx, "Setting viewport to %dx%d", width, height );

    auto realResolution = vec2( (Real)width, (Real)height );
    camera_->setViewport( realResolution );

    glViewport( 0, 0, (GLsizei)width, (GLsizei)height );
  }

  void Gfx::processEvents()
  {
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
    }
  }

  void Gfx::tick( GameTime tick, GameTime time )
  {
    camera_->update( tick, time );
  }

  void Gfx::postUpdate( GameTime delta, GameTime time )
  {
    // activate as current context
    window_->setActive( true );

    if ( flags_.resized )
    {
      renderer_->reset( viewport_.size_.x, viewport_.size_.y );
      flags_.resized = false;
      window_->setActive( true );
    }

    renderer_->draw( camera_ );

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
    camera_.reset();

    platform::RenderWindowHandler::get().setWindow( nullptr, nullptr );

    window_->close();
    window_.reset();

    platform::RenderWindowHandler::free();
  }

  void Gfx::restart()
  {
    shutdown();
    preInitialize();
    postInitialize();
  }

  Gfx::~Gfx()
  {
    shutdown();
  }

  const string c_gfxThreadName = "nekoRenderer";

  ThreadedRenderer::ThreadedRenderer( ThreadedLoaderPtr loader, FontManagerPtr fonts, MessagingPtr messaging, DirectorPtr director, ConsolePtr console )
      : loader_( move( loader ) ), fonts_( move( fonts ) ), messaging_( move( messaging ) ), director_( move( director ) ), console_( move( console ) ),
        thread_( c_gfxThreadName, threadProc, this )
  {
  }

  void ThreadedRenderer::initialize()
  {
    gfx_ = make_shared<Gfx>( loader_, fonts_, messaging_, director_, console_ );
    gfx_->postInitialize();
  }

  void ThreadedRenderer::run( platform::Event& wantStop )
  {
    while ( true )
    {
      if ( wantStop.check() )
        break;

      gfx_->processEvents();

      gfx_->preUpdate( 0.0f );

      gfx_->tick( 0.0f, 0.0f );

      gfx_->postUpdate( 0.0f, 0.0f );

      platform::sleep( 1 );
    }
  }

  bool ThreadedRenderer::threadProc( platform::Event& running, platform::Event& wantStop, void* argument )
  {
    platform::performanceInitializeRenderThread();

    auto gfx = ( (ThreadedRenderer*)argument )->shared_from_this();
    gfx->initialize();
    running.set();
    gfx->run( wantStop );

    platform::performanceTeardownCurrentThread();
    return true;
  }

  void ThreadedRenderer::start()
  {
    thread_.start();
  }

  void ThreadedRenderer::stop()
  {
    thread_.stop();
  }

  ThreadedRenderer::~ThreadedRenderer()
  {
    //
  }

  FrameQueue::FrameQueue()
  {
  }

}