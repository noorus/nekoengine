#include "stdafx.h"
#include "neko_exception.h"
#include "engine.h"
#include "gfx.h"
#include "shaders.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  namespace static_geometry {

    const vector<Vertex2D> quad2D =
    { //  x     y     s     t
      { 0.0f, 0.0f, 0.0f, 1.0f },
      { 0.0f, 1.0f, 0.0f, 0.0f },
      { 1.0f, 0.0f, 1.0f, 0.0f },
      { 1.0f, 1.0f, 1.0f, 1.0f }
    };

    const vector<PixelRGBA> image4x4 =
    {
      { 255, 0, 0, 255 },
      { 0, 255, 0, 255 },
      { 0, 0, 255, 255 },
      { 255, 0, 255, 255 }
    };

  }

  static TexturePtr g_texture;

  // Gfx

  const char cWindowTitle[] = "nekoengine-render";
  const vec4 cClearColor = vec4( 0.175f, 0.175f, 0.5f, 1.0f );

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

  void Gfx::openglDebugCallbackFunction(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam )
  {
    if ( !userParam )
      return;
    auto gfx = (Gfx*)userParam;
    gfx->engine_->console()->printf( Console::srcGfx, "OpenGL Debug: %s", message );
  }

  void Gfx::preInitialize()
  {
    info_.clear();

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::VideoMode videoMode( 1280, 720, desktop.bitsPerPixel );

    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 0;
    settings.majorVersion = 4;
    settings.minorVersion = 6;
    settings.attributeFlags = sf::ContextSettings::Attribute::Core | sf::ContextSettings::Attribute::Debug;

    window_ = make_unique<sf::Window>( videoMode, "OpenGL", sf::Style::Default, settings );

    window_->setVerticalSyncEnabled( true ); // vsync
    window_->setFramerateLimit( 0 ); // no sleep till Brooklyn

    window_->setActive( true );

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

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );

    glEnable( GL_MULTISAMPLE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_POLYGON_SMOOTH );

    glEnable( GL_DEBUG_OUTPUT );
    glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
    glDebugMessageCallback( Gfx::openglDebugCallbackFunction, this );
    glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true );
  }

  void Gfx::postInitialize()
  {
    int width, height;
    width = window_->getSize().x;
    height = window_->getSize().y;

    renderer_ = make_shared<Renderer>();

    auto realResolution = vec2( (Real)width, (Real)height );
    camera_ = make_unique<Camera>( realResolution, vec3( 0.0f, 0.0f, 0.0f ) );

    engine_->console()->printf( Console::srcGfx, "Setting viewport to %dx%d", width, height );
    glViewport( 0, 0, width, height );
    glClearColor( cClearColor.r, cClearColor.g, cClearColor.b, cClearColor.a );

    shaders_ = make_shared<Shaders>( engine_ );
    shaders_->initialize();

    meshes_ = make_shared<MeshManager>();
    auto quadVBO = meshes_->pushVBO( static_geometry::quad2D );
    meshes_->uploadVBOs();
    auto triangleVao = meshes_->pushVAO( VAO::VBO_2D, quadVBO );
    meshes_->uploadVAOs();

    g_texture = make_shared<Texture>( renderer_.get(), 2, 2, GL_RGBA8, (const void*)static_geometry::image4x4.data() );

    engine_->operationContinueVideo();
  }

  void Gfx::preUpdate( GameTime time )
  {
    //
  }

  void Gfx::tick( GameTime tick, GameTime time )
  {
    sf::Event evt;

    while ( window_->pollEvent( evt ) )
    {
      if ( evt.type == sf::Event::Closed )
        engine_->signalStop();
    }

    camera_->update( tick, time );
  }

  void Gfx::postUpdate( GameTime delta, GameTime time )
  {
    // activate as current context, just to be sure
    window_->setActive( true );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDisable( GL_DEPTH_TEST );
    glDepthMask( 0 );

    mat4 model( 1.0f );
    model = glm::scale( model, vec3( 100.0f, 100.0f, 1.0f ) );
    shaders_->setMatrices( model, camera_->view(), camera_->projection() );

    shaders_->use( 0 );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, g_texture->handle() );

    meshes_->getVAO( 0 ).draw( GL_TRIANGLE_STRIP );

    window_->display();
  }

  void Gfx::shutdown()
  {
    g_texture.reset();
    engine_->operationSuspendVideo();

    meshes_->teardown();

    shaders_->shutdown();
    shaders_.reset();

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