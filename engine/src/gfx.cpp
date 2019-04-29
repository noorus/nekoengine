#include "stdafx.h"
#include "neko_exception.h"
#include "engine.h"
#include "gfx.h"

namespace neko {

  glm::mat4 identityMatrix;
  glm::mat4 framebufferProjectionMatrix;

  const char cWindowTitle[] = "nekoengine-render";
  const vec4 cClearColor = vec4( 0.175f, 0.175f, 0.5f, 1.0f );

  Gfx::Gfx( EnginePtr engine ): Subsystem( move( engine ) ),
    window_( nullptr ), screenSurface_( nullptr ), renderer_( nullptr )
  {
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
      NEKO_EXCEPT( "SDL initialization failed" );

    identityMatrix = glm::mat4();
    framebufferProjectionMatrix = glm::ortho( 0, 1, 1, 0, -500, 500 );

    preInitialize();
  }

  void Gfx::preInitialize()
  {
    SDL_GetDesktopDisplayMode( 0, &displayMode_ );

    window_ = SDL_CreateWindow( cWindowTitle,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      1280, 720,
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

    if ( !window_ )
      NEKO_EXCEPT( "Window creation failed" );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    glContext_ = SDL_GL_CreateContext( window_ );
    if ( !glContext_ )
      NEKO_EXCEPT( "GL context creation failed" );

    if ( SDL_GL_SetSwapInterval( -1 ) == -1 )
      SDL_GL_SetSwapInterval( 1 );

    glewExperimental = GL_TRUE;
    if ( glewInit() != GLEW_OK )
      NEKO_EXCEPT( "GLEW initialization failed" );

    glClearColor( 0, 0, 0, 1 );

    glEnable( GL_TEXTURE_2D );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0 );

    screenSurface_ = SDL_GetWindowSurface( window_ );
    if ( !screenSurface_ )
      NEKO_EXCEPT( "SDL window surface fetch failed" );

    renderer_ = SDL_CreateRenderer( window_, -1, SDL_RENDERER_ACCELERATED );
    if ( !renderer_ )
      NEKO_EXCEPT( "Renderer instantiation failed" );
  }

  void Gfx::postInitialize()
  {
    int width, height;
    if ( SDL_GetRendererOutputSize( renderer_, &width, &height ) != 0 )
      NEKO_EXCEPT( "Renderer output size fetch failed" );

    engine_->operationContinueVideo();
  }

  void Gfx::preUpdate( GameTime time )
  {
    //
  }

  void Gfx::tick( GameTime tick, GameTime time )
  {
    SDL_Event evt;
    if ( SDL_PollEvent( &evt ) != 0 )
    {
      if ( evt.type == SDL_QUIT )
        engine_->signalStop();
    }
  }

  void Gfx::postUpdate( GameTime delta, GameTime time )
  {
    glClearColor( 0.0f, 1.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    SDL_GL_SwapWindow( window_ );
  }

  void Gfx::shutdown()
  {
    engine_->operationSuspendVideo();

    if ( renderer_ )
      SDL_DestroyRenderer( renderer_ );
    renderer_ = nullptr;

    if ( glContext_ )
      SDL_GL_DeleteContext( glContext_ );
    glContext_ = nullptr;

    if ( window_ )
      SDL_DestroyWindow( window_ );
    window_ = nullptr;

    screenSurface_ = nullptr;
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
    SDL_Quit();
  }

}