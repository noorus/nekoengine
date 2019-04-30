#include "stdafx.h"
#include "neko_exception.h"
#include "engine.h"
#include "gfx.h"
#include "shaders.h"

namespace neko {

  namespace static_geometry {

    const vector<Vertex2D> quad2D =
    { //  x     y     s     t
      { 0.0f, 0.0f, 0.0f, 1.0f },
      { 0.0f, 1.0f, 0.0f, 0.0f },
      { 1.0f, 1.0f, 1.0f, 0.0f },
      { 1.0f, 0.0f, 1.0f, 1.0f }
    };

  }

  // MeshManager

  size_t MeshManager::pushVBO( vector<Vertex3D> vertices )
  {
    VBO<Vertex3D> buf;
    buf.storage_.swap( vertices );
    vbos3d_.push_back( buf );
    return ( vbos3d_.size() - 1 );
  }

  size_t MeshManager::pushVBO( vector<Vertex2D> vertices )
  {
    VBO<Vertex2D> buf;
    buf.storage_.swap( vertices );
    vbos2d_.push_back( buf );
    return ( vbos2d_.size() - 1 );
  }

  template <class T>
  void vboUploadHelper( VBOVector<T>& vbos )
  {
    vector<VBO<T>*> dirties;
    for ( auto& buf : vbos )
      if ( !buf.uploaded )
        dirties.push_back( &buf );
    vector<GLuint> ids;
    ids.resize( dirties.size() );
    glGenBuffers( (GLsizei)dirties.size(), ids.data() );
    for ( size_t i = 0; i < dirties.size(); ++i )
    {
      glBindBuffer( GL_ARRAY_BUFFER, ids[i] );
      glBufferData( GL_ARRAY_BUFFER, dirties[i]->storage_.size() * sizeof( T ), dirties[i]->storage_.data(), GL_STATIC_DRAW );
      dirties[i]->id = ids[i];
      dirties[i]->uploaded = true;
    }
  }

  void MeshManager::uploadVBOs()
  {
    vboUploadHelper( vbos3d_ );
    vboUploadHelper( vbos2d_ );
  }

  size_t MeshManager::pushVAO( VAO::VBOType type, size_t verticesVBO )
  {
    if ( type == VAO::VBO_3D && ( verticesVBO >= vbos3d_.size() || !vbos3d_[verticesVBO].uploaded ) )
      NEKO_EXCEPT( "VBO3D index out of bounds or VBO not uploaded while defining VAO" );
    if ( type == VAO::VBO_2D && ( verticesVBO >= vbos2d_.size() || !vbos2d_[verticesVBO].uploaded ) )
      NEKO_EXCEPT( "VBO2D index out of bounds or VBO not uploaded while defining VAO" );
    vaos_.emplace_back( type, verticesVBO );
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
      if ( dirties[i]->vboType_ == VAO::VBO_3D )
      {
        auto vbo = &( vbos3d_[dirties[i]->vbo_] );
        if ( !vbo->uploaded )
          NEKO_EXCEPT( "VBO3D used for VAO has not been uploaded" );
        glBindVertexArray( ids[i] );
        glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex3D ), nullptr ); // x, y, z
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex3D ), (void*)( 3 * sizeof( float ) ) ); // s, t
        dirties[i]->id = ids[i];
        dirties[i]->size_ = vbo->storage_.size();
        dirties[i]->uploaded_ = true;
      }
      else if ( dirties[i]->vboType_ == VAO::VBO_2D )
      {
        auto vbo = &( vbos2d_[dirties[i]->vbo_] );
        if ( !vbo->uploaded )
          NEKO_EXCEPT( "VBO2D used for VAO has not been uploaded" );
        glBindVertexArray( ids[i] );
        glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), nullptr ); // x, y
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), (void*)( 2 * sizeof( float ) ) ); // s, t
        dirties[i]->id = ids[i];
        dirties[i]->size_ = vbo->storage_.size();
        dirties[i]->uploaded_ = true;
      }
    }
    glBindVertexArray( 0 );
  }

  void VAO::draw( GLenum mode )
  {
    glBindVertexArray( id );
    glDrawArrays( mode, 0, (GLsizei)size_ );
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

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );

    SDL_GL_SetSwapInterval( 1 ); // vsync

    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
      NEKO_EXCEPT( "SDL initialization failed" );

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

    screenSurface_ = SDL_GetWindowSurface( window_ );
    if ( !screenSurface_ )
      NEKO_EXCEPT( "SDL window surface fetch failed" );
  }

  void Gfx::postInitialize()
  {
    int width, height;
    width = screenSurface_->w;
    height = screenSurface_->h;

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
    camera_->update( tick, time );
  }

  void Gfx::postUpdate( GameTime delta, GameTime time )
  {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDisable( GL_DEPTH_TEST );
    glDepthMask( 0 );

    mat4 model( 1.0f );
    model = glm::scale( model, vec3( 100.0f, 100.0f, 1.0f ) );
    shaders_->setMatrices( model, camera_->view(), camera_->projection() );

    shaders_->use( 0 );
    meshes_->getVAO( 0 ).draw( GL_QUADS );

    SDL_GL_SwapWindow( window_ );
  }

  void Gfx::shutdown()
  {
    engine_->operationSuspendVideo();

    meshes_->teardown();

    shaders_->shutdown();
    shaders_.reset();

    camera_.reset();

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