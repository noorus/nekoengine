#include "stdafx.h"
#include "neko_exception.h"
#include "engine.h"
#include "gfx.h"
#include "shaders.h"
#include "renderer.h"
#include "camera.h"
#include "fontmanager.h"
#include "console.h"

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

  Gfx::Gfx( EnginePtr engine ): Subsystem( move( engine ) )
  {
    preInitialize();
  }

  void Gfx::printInfo()
  {
    engine_->console()->print( Console::srcGfx, "GL Vendor: " + info_.vendor_ );
    engine_->console()->print( Console::srcGfx, "GL Version: " + info_.version_ );
    engine_->console()->print( Console::srcGfx, "GL Renderer: " + info_.renderer_ );
  }

  // 131185: Buffer detailed info...
  // 131218: Program/shader state performance warning...
  // 131204: The texture object does not have a defined base level...
  const std::array<GLuint, 3> c_ignoredGlDebugMessages = { 131185, 131218, 131204 };

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
      gfx->engine_->console()->printf( Console::srcGfx, "OpenGL Debug: %s (%d)", message, id );
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

  class WindowProcDetour: boost::noncopyable {
  public:
    HWND window_;
    WNDPROC original_;
    explicit WindowProcDetour( HWND window, WNDPROC newProc ): window_( window )
    {
      original_ = reinterpret_cast<WNDPROC>( GetWindowLongPtrW( window_, GWLP_WNDPROC ) );
      SetWindowLongPtrW( window_, GWLP_WNDPROC, reinterpret_cast<DWORD_PTR>( newProc ) );
    }
    ~WindowProcDetour()
    {
      SetWindowLongPtrW( window_, GWLP_WNDPROC, reinterpret_cast<DWORD_PTR>( original_ ) );
    }
  };

  WindowProcDetour* detour = nullptr;

  const auto targetResolution = vec2i( 1920, 1080 );

  void getWindowBordersSize( HWND wnd, SIZE& size, bool withClient )
  {
    auto style = (DWORD)GetWindowLongW( wnd, GWL_STYLE );
    auto exstyle = (DWORD)GetWindowLongW( wnd, GWL_EXSTYLE );
    RECT want;
    want.left = 100;
    want.right = 100 + targetResolution.x;
    want.top = 100;
    want.bottom = 100 + targetResolution.y;
    AdjustWindowRectEx( &want, style, FALSE, exstyle );
    size.cx = ( want.right - want.left - targetResolution.x );
    size.cy = ( want.bottom - want.top - targetResolution.y );
  }

  LRESULT wndProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
  {
    const float vaspect = ( (float)targetResolution.x / (float)targetResolution.y );
    const float haspect = ( (float)targetResolution.y / (float)targetResolution.x );
    if ( msg == WM_SIZING )
    {
      SIZE borders;
      getWindowBordersSize( wnd, borders, true );
      auto rekt = (RECT*)lparam;
      auto dw = float( rekt->right - rekt->left );
      auto dh = float( rekt->bottom - rekt->top );
      auto w = dw - borders.cx;
      auto h = dh - borders.cy;
      bool sel = false;
      if ( wparam == WMSZ_LEFT || wparam == WMSZ_RIGHT )
        sel = true;
      else if ( wparam == WMSZ_TOP || wparam == WMSZ_BOTTOM )
        sel = false;
      else
        sel = ( ( w / h ) > vaspect );
      float nw = w, nh = h;
      if ( sel )
        nh = ( w * haspect );
      else
        nw = ( h * vaspect );
      nw += borders.cx;
      nh += borders.cy;
      if ( wparam == WMSZ_TOPLEFT || wparam == WMSZ_TOPRIGHT )
      {
        if ( wparam == WMSZ_TOPRIGHT )
          rekt->right = rekt->left + (LONG)math::round( nw );
        else
          rekt->left = rekt->right - (LONG)math::round( nw );
        rekt->top = rekt->bottom - (LONG)math::round( nh );
        return TRUE;
      }
      switch ( wparam )
      {
        case WMSZ_TOP:
        case WMSZ_BOTTOM:
          rekt->left = (LONG)math::round( (float)rekt->left + ( ( dw - nw ) / 2.0f ) );
        break;
        case WMSZ_BOTTOMLEFT:
          rekt->left = (LONG)math::round( (float)rekt->right - nw );
          rekt->top = sel ? rekt->top : (LONG)math::round( (float)rekt->bottom - nh );
        break;
      }
      rekt->right = rekt->left + (LONG)math::round( nw );
      rekt->bottom = rekt->top + (LONG)math::round( nh );
      return TRUE;
    }
    return detour->original_( wnd, msg, wparam, lparam );
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
    auto handle = window_->getSystemHandle();
    detour = new WindowProcDetour( handle, wndProc );

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
    renderer_ = make_shared<Renderer>( engine_ );
    renderer_->preInitialize();

    setOpenGLDebugLogging( g_CVar_gl_debuglog.as_b() );

    auto realResolution = vec2( (Real)window_->getSize().x, (Real)window_->getSize().y );
    camera_ = make_unique<Camera>( realResolution, vec3( 0.0f, 0.0f, 0.0f ) );

    resize( window_->getSize().x, window_->getSize().y );

    renderer_->initialize( viewport_.size_.x, viewport_.size_.y );

    engine_->operationContinueVideo();
  }

  void Gfx::preUpdate( GameTime time )
  {
    renderer_->prepare( time );
  }

  void Gfx::resize( size_t width, size_t height )
  {
    viewport_.size_ = vec2i( width, height );

    engine_->console()->printf( Console::srcGfx, "Setting viewport to %dx%d", width, height );

    auto realResolution = vec2( (Real)width, (Real)height );
    camera_->setViewport( realResolution );

    glViewport( 0, 0, (GLsizei)width, (GLsizei)height );
  }

  void Gfx::tick( GameTime tick, GameTime time )
  {
    sf::Event evt;

    while ( window_->pollEvent( evt ) )
    {
      if ( evt.type == sf::Event::Closed )
        engine_->signalStop();
      else if ( evt.type == sf::Event::Resized )
      {
        resize( evt.size.width, evt.size.height );
        flags_.resized = true;
      }
    }

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

  void Gfx::shutdown()
  {
    engine_->operationSuspendVideo();

    camera_.reset();

    window_->close();
    window_.reset();
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

}