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

#pragma comment( lib, "opengl32.lib" )

#ifdef _DEBUG
# pragma comment( lib, "glbindingd.lib" )
# pragma comment( lib, "glbinding-auxd.lib" )
# pragma comment( lib, "sfml-system-s-d.lib" )
# pragma comment( lib, "sfml-window-s-d.lib" )
# ifndef NEKO_NO_GUI
#  pragma comment( lib, "MyGUIEngine_d.lib" )
# endif
#else
# pragma comment( lib, "glbinding.lib" )
# pragma comment( lib, "sfml-system-s.lib" )
# pragma comment( lib, "sfml-window-s.lib" )
# ifndef NEKO_NO_GUI
#  pragma comment( lib, "MyGUIEngine.lib" )
# endif
#endif

namespace neko {

  using namespace gl;

  const char c_windowTitle[] = "nekoengine//render"
#ifdef _DEBUG
    " [debug]";
#else
    " [release]";
#endif

  const char c_imguiGlslVersion[] = "#version 450";

  NEKO_DECLARE_CONVAR( vid_screenwidth, "Screen width. Changes are applied when the renderer is restarted.", 1920 );
  NEKO_DECLARE_CONVAR( vid_screenheight, "Screen height. Changes are applied when the renderer is restarted.", 1080 );

  NEKO_DECLARE_CONVAR( vid_viewportwidth, "Game viewport native width. Changes are applied when the renderer is restarted.", 1280 );
  NEKO_DECLARE_CONVAR( vid_viewportheight, "Game viewport native height. Changes are applied when the renderer is restarted.", 720 );

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

#ifdef IMGUI_VERSION
    console_->print( Console::srcGfx, "Using Dear ImGui v" IMGUI_VERSION );
#endif
  }

  // 8: (Intel) Redundant state change in glBindFramebuffer API call, FBO 0, "", already bound.
  // 131185: Buffer detailed info...
  // -> Information on buffer object creation, hinting and memory placement
  // 131218: Program/shader state performance warning...
  // 131204: The texture object does not have a defined base level...
  // 131154: Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
  // -> Basically just means that we're not using NV-specific (?) multithreaded texture uploads
  // -> https://on-demand.gputechconf.com/gtc/2012/presentations/S0356-GTC2012-Texture-Transfers.pdf
  // 131139: Rasterization quality warning: A non-fullscreen clear caused a fallback from CSAA to MSAA.
  // -> If glClear is used when scissor testing or glViewport causes the area to be limited
  const array<GLuint, 6> c_ignoredGlDebugMessages = { 8, 131185, 131218, 131204, 131154, 131139 };

  void Gfx::openglDebugCallbackFunction( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam )
  {
    for ( GLuint ignoredId : c_ignoredGlDebugMessages )
      if ( id == ignoredId )
        return;

    auto gfx = static_cast<Gfx*>( const_cast<void*>( userParam ) );

  #ifdef _DEBUG
    if ( id == 0
      // || id == 8
      || id == 1281
      || id == 1282
      || id == 1285
      || id == 131168
         // || id == 131076
      )
      DebugBreak();
  #endif

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
    sf::VideoMode videoMode( sf::Vector2u( g_CVar_vid_screenwidth.as_i(), g_CVar_vid_screenheight.as_i() ), desktop.bitsPerPixel );

    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 0;
    settings.sRgbCapable = false;
    settings.majorVersion = c_glVersion[0];
    settings.minorVersion = c_glVersion[1];
    settings.attributeFlags = sf::ContextSettings::Attribute::Core;

    if ( g_CVar_gl_debuglog.as_i() > 1 )
      settings.attributeFlags |= sf::ContextSettings::Attribute::Debug;

    window_ = make_unique<sf::Window>( videoMode, c_windowTitle, sf::Style::Default, settings );

    platform::setWindowIcon( window_->getSystemHandle(), platform::KnownIcon::MainIcon );

    window_->setVerticalSyncEnabled( g_CVar_vid_vsync.as_b() ); // vsync
    window_->setFramerateLimit( 0 ); // no sleep till Brooklyn

    if ( !window_->setActive( true ) )
      window_->requestFocus();

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

  static const vector<EditorViewportDefinition> g_editorViewportDefs = {
    { .name = "top", .eye = vec3( 0.01f, -100.0f, 0.0f ), .up = vec3( 0.0f, 1.0f, 0.0f ) },
    { .name = "front", .eye = vec3( 0.0f, 0.01f, 100.0f ), .up = vec3( 0.0f, 1.0f, 0.0f ) },
    { .name = "left", .eye = vec3( -100.0f, 0.01f, 0.0f ), .up = vec3( 0.0f, 1.0f, 0.0f ) }
  };

  EditorViewport::EditorViewport( SceneManager* manager, vec2 resolution, const EditorViewportDefinition& def )
  {
    camera_ = make_unique<EditorOrthoCamera>( manager, resolution, def );
  }

  void Gfx::postInitialize( Engine& engine )
  {
    renderer_ = make_shared<Renderer>( loader_, fonts_, director_, console_ );
    renderer_->preInitialize();
    target_ = renderer_->createSceneNode( nullptr );

    setOpenGLDebugLogging( g_CVar_gl_debuglog.as_i() > 0 );

    auto realResolution = vec2( (Real)window_->getSize().x, (Real)window_->getSize().y );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& igIO = ImGui::GetIO();
    igIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    igIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //igIO.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
    //igIO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();

    auto& igStyle = ImGui::GetStyle();
    if ( igIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
    {
      igStyle.WindowRounding = 0.0f;
      igStyle.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplWin32_Init( window_->getSystemHandle() );
    ImGui_ImplOpenGL3_Init( c_imguiGlslVersion );

    camera_ = make_unique<OrbitCamera>( renderer_.get(), realResolution, target_,
    vec3( 5.0f, 3.0f, 5.0f ), // vecOffset
    60.0f, // fov
    true, // reverse
    10.0f, // sens
    5.0f, // mindist
    50.0f, // maxdist
    2.0f, // rotdecel
    5.0f, // zoomaccel
    2.0f ); // zoomdecel

    for ( const auto& def : g_editorViewportDefs )
    {
      EditorViewport vp( renderer_.get(), realResolution, def );
      viewports_.push_back( move( vp ) );
    }

    resize( window_->getSize().x, window_->getSize().y );

    renderer_->initialize( gameViewport_.size_.x, gameViewport_.size_.y );

    auto documentsPath = platform::wideToUtf8( engine.env().documentsPath_ );

    #ifndef NEKO_NO_GUI
    gui_->initialize( this, documentsPath, *window_ );
    gui_->setInstallationInfo( engine.installationInfo() );
    #endif

    input_->initialize( window_->getSystemHandle() );

    messaging_->listen( this );
  }

  void Gfx::resize( size_t width, size_t height )
  {
    auto newfbosize = vec2i( g_CVar_vid_viewportwidth.as_i(), g_CVar_vid_viewportheight.as_i() );
    if ( newfbosize != gameViewport_.size_ )
    {
      console_->printf( Console::srcGfx, "Resizing main fbo to %dx%d", newfbosize.x, newfbosize.y );
      gameViewport_.size_ = newfbosize;
      flags_.mainbufResized = true;
    }

    auto newwindowsize = vec2i( width, height );
    if ( newwindowsize != windowViewport_.size_ )
    {
      console_->printf( Console::srcGfx, "Resizing viewport to %dx%d", newwindowsize.x, newwindowsize.y );
      windowViewport_.size_ = newwindowsize;
      flags_.editorResized = true;
    }
    input_->setWindowSize( windowViewport_.size_ );


    for ( auto& viewport :viewports_)
    {
      viewport.size_ = vec2i( static_cast<GLsizei>( width / 2 ), static_cast<GLsizei>( height / 2 ) );
      viewport.camera()->setViewport( vec2( static_cast<Real>( width ) * 0.5f, static_cast<Real>( height ) * 0.5f ) );
    }

    auto realResolution = vec2( (Real)width, (Real)height );
    camera_->setViewport( realResolution );

    glViewport( 0, 0, static_cast<GLsizei>( width ), static_cast<GLsizei>( height ) );

    #ifndef NEKO_NO_GUI
    gui_->resize( static_cast<int>( width ), static_cast<int>( height ) );
    #endif
  }

  void Gfx::processEvents()
  {
    input_->update();

    sf::Event evt {};
    while ( window_->pollEvent( evt ) )
    {
      if ( evt.type == sf::Event::Closed )
      {
        messaging_->send( M_Window_Close );
      }
      else if ( evt.type == sf::Event::Resized )
      {
        resize( evt.size.width, evt.size.height );
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
        if ( evt.key.code == sf::Keyboard::F7 )
          messaging_->send( M_Debug_PauseTime );
        if ( evt.key.code == sf::Keyboard::F8 )
          messaging_->send( M_Debug_ToggleDevMode );
        if ( evt.key.code == sf::Keyboard::F9 )
        {
          messaging_->send( M_Debug_ToggleWireframe );
          g_CVar_dbg_wireframe.toggle();
        }
      }
    }
  }

  void Gfx::onMessage( const Message& msg )
  {
    logicLock_.lock();
    if ( msg.code == M_Extern_AccountUpdated )
    {
      auto id = (size_t)( msg.arguments[0] );
      updateAccounts_.push( id );
    }
    logicLock_.unlock();
  }

  // clang-format off

  static ViewportDrawParameters g_editorparams = {
    .drawSky = false,
    .drawFBO = true,
    .drawWireframe = false,
    .isEditor = true,
    .fullWindowResolution = { 0.0f, 0.0f }
  };

  static ViewportDrawParameters g_liveparams = {
    .drawSky = true,
    .drawFBO = true,
    .drawWireframe = false,
    .isEditor = false,
    .fullWindowResolution = { 0.0f, 0.0f }
  };

  // clang-format on

  void Gfx::updateRealTime( GameTime realTime, GameTime delta, Engine& engine )
  {
    auto& igIO = ImGui::GetIO();

    int vp = 3;
    if ( engine.devmode() )
    {
      const auto halfwidth = static_cast<GLsizei>( windowViewport_.size_.x * 0.5f );
      const auto halfheight = static_cast<GLsizei>( windowViewport_.size_.y * 0.5f );
      if ( input_->mousePosition_.x < halfwidth )
        vp = ( input_->mousePosition_.y < halfheight ? 0 : 2 );
      else
        vp = ( input_->mousePosition_.y < halfheight ? 1 : 3 );
    }

    if ( vp == 3 )
    {
      if ( input_->mousebtn( 2 ) )
        camera_->applyInputPanning( input_->movement() );
      else if ( input_->mousebtn( 1 ) )
        camera_->applyInputRotation( input_->movement() );

      camera_->applyInputZoom( static_cast<int>( input_->movement().z ) );
    }
    else
    {
      if ( input_->mousebtn( 2 ) )
        viewports_[vp].camera()->applyInputPanning( input_->movement() );
      viewports_[vp].camera()->applyInputZoom( static_cast<int>( input_->movement().z ) );
    }

    for ( auto& vp : viewports_ )
      vp.camera()->update( delta, realTime );

    camera_->update( delta, realTime );
  }

  static bool show_demo_window = true;

  void Gfx::update( GameTime time, GameTime delta, Engine& engine )
  {
    if ( flags_.reloadShaders )
    {
      renderer_->shaders().shutdown();
      renderer_->shaders().initialize();
      flags_.reloadShaders = false;
    }

    renderer_->update( delta, time );

    {
      char stats[256];
      auto secondsWasted = chrono::seconds( static_cast<int>( engine.stats().f_timeWasted.load() + (float)time ) );
      sprintf_s( stats, 256,
        "Launches: %i\nTime wasted: %s", engine.stats().i_launches.load(),
        utils::beautifyDuration( secondsWasted ).c_str() );
      gui_->setDebugStats( stats );
    }

    // activate as current context
    ignore = window_->setActive( true );

#ifndef NEKO_NO_RAINET
    while ( !updateAccounts_.empty() )
    {
      auto id = updateAccounts_.front();
      updateAccounts_.pop();
      auto account = engine.rainet()->account( id );
      if ( !account )
        continue;
      if ( account->id_ == engine.rainet()->accountMe()->id_ && account->steamImage_ )
        renderer_->setUserData( account->id_, account->steamName_, *account->steamImage_ );
    }
#endif

    if ( flags_.mainbufResized )
    {
      renderer_->reset( gameViewport_.size_.x, gameViewport_.size_.y );
      flags_.mainbufResized = false;
      ignore = window_->setActive( true );
    }

#ifndef NEKO_NO_GUI
    renderer_->draw( time, delta, *camera_.get(), gui_->platform() );
#else

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    glClearColor( 0.0f, 0.1f, 0.2f, 1.0f );
    glDepthMask( TRUE );
    glDepthFunc( GL_LESS );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    g_editorparams.fullWindowResolution = windowViewport_.size_;
    g_liveparams.fullWindowResolution = windowViewport_.size_;

    if ( engine.devmode() )
    {
      const auto halfwidth = static_cast<GLsizei>( windowViewport_.size_.x * 0.5f );
      const auto halfheight = static_cast<GLsizei>( windowViewport_.size_.y * 0.5f );
      for ( int i = 0; i < 3; ++i )
      {
        renderer_->draw( time, delta, *viewports_[i].camera(), g_editorparams, renderer_->builtins().screenFourthQuads_[i] );
      }
      renderer_->draw( time, delta, *camera_, g_liveparams, renderer_->builtins().screenFourthQuads_[3] );
    }
    else
    {
      renderer_->draw( time, delta, *camera_, g_liveparams, renderer_->builtins().screenQuad_ );
    }

    if ( show_demo_window )
      ImGui::ShowDemoWindow( &show_demo_window );

    auto gameMainTexture = renderer_->getMergedMainFramebuffer();
    if ( gameMainTexture )
    {
      auto wndsize = ImVec2( gameViewport_.sizef() );
      ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
      ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
      ImGui::SetNextWindowContentSize( wndsize );
      ImGui::SetNextWindowSizeConstraints( wndsize, wndsize );
      ImGui::Begin( "Game", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
          ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking );
      ImGui::Image( reinterpret_cast<ImTextureID>( static_cast<intptr_t>( gameMainTexture->handle() ) ), wndsize,
        ImVec2( 0, 1 ), ImVec2( 1, 0 ) );
      ImGui::End();
      ImGui::PopStyleVar( 2 );
    }

    ImGui::EndFrame();

#endif

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
    if ( ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
    {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
    }

    window_->display();
  }

  const Image& Gfx::renderWindowReadPixels()
  {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    lastCapture_.size_ = size2i( windowViewport_.size_ );
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

    for ( auto& vp : viewports_ )
      vp.camera().reset();

    camera_.reset();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    platform::RenderWindowHandler::get().setWindow( nullptr, nullptr );

    window_->close();
    window_.reset();

    platform::RenderWindowHandler::free();
  }

  void Gfx::jsRestart()
  {
    renderer_->jsRestart();
    renderer_->update( 0.0, 0.0 );
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

}