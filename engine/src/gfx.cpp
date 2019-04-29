#include "stdafx.h"
#include "neko_exception.h"
#include "engine.h"
#include "gfx.h"
#include "shaders.h"

namespace neko {

  // MeshManager

  size_t MeshManager::pushVBO( vector<Vertex> vertices )
  {
    VBO<Vertex> buf;
    buf.storage_.swap( vertices );
    buffers_.push_back( buf );
    return ( buffers_.size() - 1 );
  }

  void MeshManager::uploadVBOs()
  {
    vector<VBO<Vertex>*> dirties;
    for ( auto& buf : buffers_ )
      if ( !buf.uploaded )
        dirties.push_back( &buf );
    vector<GLuint> ids;
    ids.resize( dirties.size() );
    glGenBuffers( (GLsizei)dirties.size(), ids.data() );
    for ( size_t i = 0; i < dirties.size(); ++i )
    {
      glBindBuffer( GL_ARRAY_BUFFER, ids[i] );
      glBufferData( GL_ARRAY_BUFFER, dirties[i]->storage_.size() * sizeof( Vertex ), dirties[i]->storage_.data(), GL_STATIC_DRAW );
      dirties[i]->id = ids[i];
      dirties[i]->uploaded = true;
    }
  }

  size_t MeshManager::pushVAO( size_t verticesVBO )
  {
    if ( verticesVBO >= buffers_.size() || !buffers_[verticesVBO].uploaded )
      NEKO_EXCEPT( "VBO index out of bounds or VBO not uploaded while defining VAO" );
    vaos_.emplace_back( verticesVBO );
    return ( vaos_.size() - 1 );
  }

  void MeshManager::uploadVAOs()
  {
    vector<VAO*> dirties;
    for ( auto& vao : vaos_ )
      if ( !vao.uploaded_ )
        dirties.push_back( &vao );
    vector<GLuint> ids;
    ids.resize( dirties.size() );
    glGenVertexArrays( (GLsizei)dirties.size(), ids.data() );
    for ( size_t i = 0; i < dirties.size(); ++i )
    {
      auto vbo = &( buffers_[dirties[i]->vbo_] );
      if ( !vbo->uploaded )
        NEKO_EXCEPT( "VBO used for VAO has not been uploaded" );
      glBindVertexArray( ids[i] );
      glEnableVertexAttribArray( 0 );
      glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
      glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
      dirties[i]->id = ids[i];
      dirties[i]->size_ = vbo->storage_.size();
      dirties[i]->uploaded_ = true;
    }
  }

  void VAO::draw()
  {
    glBindVertexArray( id );
    glDrawArrays( GL_TRIANGLES, 0, (GLsizei)size_ );
  }

  void MeshManager::teardown()
  {
    //
  }

  // Gfx

  const char cWindowTitle[] = "nekoengine-render";
  const vec4 cClearColor = vec4( 0.175f, 0.175f, 0.5f, 1.0f );

  Gfx::Gfx( EnginePtr engine ): Subsystem( move( engine ) ),
    window_( nullptr ), screenSurface_( nullptr )
  {
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 5 );

    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 ); // native depth buffer bits

    SDL_GL_SetSwapInterval( 1 ); // vsync

    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
      NEKO_EXCEPT( "SDL initialization failed" );

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

    glContext_ = SDL_GL_CreateContext( window_ );
    if ( !glContext_ )
      NEKO_EXCEPT( "GL context creation failed" );

    if ( SDL_GL_SetSwapInterval( -1 ) == -1 )
      SDL_GL_SetSwapInterval( 1 );

    glewExperimental = GL_TRUE;
    if ( glewInit() != GLEW_OK )
      NEKO_EXCEPT( "GLEW initialization failed" );

    auto renderer = glGetString( GL_RENDERER );
    if ( renderer )
      engine_->console()->printf( Console::srcGfx, "Renderer: %s", renderer );

    auto version = glGetString( GL_VERSION );
    if ( version )
      engine_->console()->printf( Console::srcGfx, "Version: %s", version );

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );

    glEnable( GL_MULTISAMPLE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_POLYGON_SMOOTH );

    screenSurface_ = SDL_GetWindowSurface( window_ );
    if ( !screenSurface_ )
      NEKO_EXCEPT( "SDL window surface fetch failed" );
  }

  void Gfx::postInitialize()
  {
    int width, height;
    width = screenSurface_->w;
    height = screenSurface_->h;

    glViewport( 0, 0, width, height );
    glClearColor( 0.125f, 0.125f, 0.125f, 1.0f );

    shaders_ = make_shared<Shaders>( engine_ );
    shaders_->initialize();

    meshes_ = make_shared<MeshManager>();
    auto triangleVbo = meshes_->pushVBO({
      { 0.0f, 0.5f, 0.0f },
      { 0.5f, -0.5f, 0.0f },
      { -0.5f, -0.5f, 0.0f }
    });
    meshes_->uploadVBOs();
    auto triangleVao = meshes_->pushVAO( triangleVbo );
    meshes_->uploadVAOs();

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
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    shaders_->use( 0 );
    meshes_->getVAO( 0 ).draw();

    SDL_GL_SwapWindow( window_ );
  }

  void Gfx::shutdown()
  {
    engine_->operationSuspendVideo();

    meshes_->teardown();

    shaders_->shutdown();
    shaders_.reset();

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