#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"
#include "meshmanager.h"
#include "shaders.h"
#include "camera.h"
#include "loader.h"
#include "engine.h"

namespace neko {

  using namespace gl;

  namespace static_geometry {

    const vector<PixelRGBA> image4x4 =
    {
      { 255, 0, 0, 255 },
      { 0, 255, 0, 255 },
      { 0, 0, 255, 255 },
      { 255, 0, 255, 255 }
    };

    /*const vector<GLuint> quadIndexes =
    {
      0, 1, 2, 2, 3, 0
    };*/

    const vector<Vertex2D> quad2D =
    { // x      y     s     t
    { -0.5f,  0.5f, 0.0f, 0.0f }, // 0
    {  0.5f,  0.5f, 1.0f, 0.0f }, // 1
    {  0.5f, -0.5f, 1.0f, 1.0f }, // 3
    { -0.5f, -0.5f, 0.0f, 1.0f }, // 4
    { -0.5f,  0.5f, 0.0f, 0.0f }  // 5
    };

    const vector<Vertex2D> quadStrip2D =
    {   // x      y     s     t
      { -0.5f, -0.5f, 0.0f, 1.0f },
      {  0.5f, -0.5f, 1.0f, 1.0f },
      { -0.5f,  0.5f, 0.0f, 0.0f },
      {  0.5f,  0.5f, 1.0f, 0.0f },
    };

    const vector<Vertex2D> screenQuad =
    {   // x      y     s     t
      { -1.0f,  1.0f, 0.0f, 1.0f },
      { -1.0f, -1.0f, 0.0f, 0.0f },
      {  1.0f, -1.0f, 1.0f, 0.0f },
      { -1.0f,  1.0f, 0.0f, 1.0f },
      {  1.0f, -1.0f, 1.0f, 0.0f },
      {  1.0f,  1.0f, 1.0f, 1.0f },
    };

  }

  static TexturePtr g_texture;
  static FramebufferPtr g_framebuf;

  void glFetchInformation( GLInformation& info )
  {
    auto glvGetI32 = []( GLenum e, int32_t& target )
    {
      GLint data = 0;
      glGetIntegerv( e, &data );
      target = (int32_t)data;
    };
    auto glvGetI64 = []( GLenum e, int64_t& target )
    {
      GLint64 data = 0;
      glGetInteger64v( e, &data );
      target = (int64_t)data;
    };

    // OpenGL version
    glvGetI32( GL_MAJOR_VERSION, info.versionMajor );
    glvGetI32( GL_MINOR_VERSION, info.versionMinor );

    // Max buffer sizes
    glvGetI64( GL_MAX_TEXTURE_SIZE, info.maxTextureSize );
    glvGetI64( GL_MAX_RENDERBUFFER_SIZE, info.maxRenderbufferSize );
    glvGetI64( GL_MAX_FRAMEBUFFER_WIDTH, info.maxFramebufferWidth );
    glvGetI64( GL_MAX_FRAMEBUFFER_HEIGHT, info.maxFramebufferHeight );
  }

  Renderer::Renderer( EnginePtr engine ): engine_( move( engine ) )
  {
    glFetchInformation( info_ );

    if ( info_.versionMajor < 3 || ( info_.versionMajor == 3 && info_.versionMinor < 3 ) )
      NEKO_EXCEPT( "Insufficient OpenGL version" );

    shaders_ = make_shared<Shaders>( engine_ );
    shaders_->initialize();

    meshes_ = make_shared<MeshManager>();
    auto quadVBO = meshes_->pushVBO( static_geometry::quadStrip2D );
    auto screenVBO = meshes_->pushVBO( static_geometry::screenQuad );
    meshes_->uploadVBOs();
    auto triangleVao = meshes_->pushVAO( VAO::VBO_2D, quadVBO );
    auto screenVAO = meshes_->pushVAO( VAO::VBO_2D, screenVBO );
    meshes_->uploadVAOs();
    /*auto quadEBO = meshes_->pushEBO( static_geometry::quadIndexes );
    meshes_->uploadEBOs();*/

    g_texture = make_shared<Texture>( this, 2, 2, Surface::PixFmtColorRGBA8, (const void*)static_geometry::image4x4.data() );
  }

  void Renderer::initialize()
  {
    MaterialPtr myMat = make_shared<Material>();
    materials_.push_back( myMat );
    engine_->loader()->addLoadTask( { LoadTask( myMat, R"(data\textures\test.png)" ) } );

    g_framebuf = make_shared<Framebuffer>( this );
    g_framebuf->recreate( 1280, 720 );
  }

  void Renderer::uploadTextures()
  {
    MaterialVector mats;
    engine_->loader()->getFinishedMaterials( mats );
    if ( !mats.empty() )
      engine_->console()->printf( Console::srcGfx, "Renderer::uploadTextures got %d new images", mats.size() );
    for ( auto& mat : mats )
    {
      if ( !mat->loaded_ )
        continue;
      mat->texture_ = make_shared<Texture>( this, mat->image_.width_, mat->image_.height_, mat->image_.format_, mat->image_.data_.data() );
    }
  }

  //! Called by Texture::Texture()
  GLuint Renderer::implCreateTexture( size_t width, size_t height, GLGraphicsFormat format, GLGraphicsFormat internalFormat, GLGraphicsFormat internalType, const void* data )
  {
    assert( width <= (size_t)info_.maxTextureSize && height <= (size_t)info_.maxTextureSize );

    GLuint handle = 0;
    glGenTextures( 1, &handle );
    assert( handle != 0 );

    glBindTexture( GL_TEXTURE_2D, handle );

    // assumptions for now
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    // assumptions for now
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // 1 byte alignment - i.e. unaligned.
    // could boost performance to use aligned memory in the future.
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glTexImage2D( GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, internalFormat, internalType, data );

    glGenerateMipmap( GL_TEXTURE_2D );

    glBindTexture( GL_TEXTURE_2D, 0 );

    return handle;
  }

  //! Called by Texture::~Texture()
  void Renderer::implDeleteTexture( GLuint handle )
  {
    assert( handle );
    glDeleteTextures( 1, &handle );
  }

  //! Called by Renderbuffer::Renderbuffer()
  GLuint Renderer::implCreateRenderbuffer( size_t width, size_t height, GLGraphicsFormat format )
  {
    assert( width <= (size_t)info_.maxRenderbufferSize && height <= (size_t)info_.maxRenderbufferSize );

    GLuint handle = 0;
    glGenRenderbuffers( 1, &handle );
    assert( handle != 0 );

    glBindRenderbuffer( GL_RENDERBUFFER, handle );
    glRenderbufferStorage( GL_RENDERBUFFER, format, (GLsizei)width, (GLsizei)height );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );

    return handle;
  }

  //! Called by Renderbuffer::~Renderbuffer()
  void Renderer::implDeleteRenderbuffer( GLuint handle )
  {
    assert( handle );
    glDeleteRenderbuffers( 1, &handle );
  }

  //! Called by Framebuffer::create()
  GLuint Renderer::implCreateFramebuffer( size_t width, size_t height )
  {
    assert( width <= (size_t)info_.maxFramebufferWidth && height <= (size_t)info_.maxFramebufferHeight );

    GLuint handle = 0;
    glGenFramebuffers( 1, &handle );
    assert( handle != 0 );

    return handle;
  }

  //! Called by Framebuffer::destroy()
  void Renderer::implDeleteFramebuffer( GLuint handle )
  {
    assert( handle );
    glDeleteFramebuffers( 1, &handle );
  }

  void Renderer::sceneDraw( CameraPtr camera )
  {
    glDisable( GL_DEPTH_TEST );
    glDepthMask( 0 );

    mat4 model( 1.0f );
    model = glm::scale( model, vec3( 256.0f, 256.0f, 1.0f ) );
    model = glm::translate( model, vec3( 1.0f, 1.0f, 0.0f ) );
    shaders_->setMatrices( model, camera->view(), camera->projection() );

    shaders_->use( 0 );

    glActiveTexture( GL_TEXTURE0 );
    if ( materials_[0]->texture_ )
      glBindTexture( GL_TEXTURE_2D, materials_[0]->texture_->handle() );
    else
      glBindTexture( GL_TEXTURE_2D, g_texture->handle() );

    meshes_->getVAO( 0 ).draw( GL_TRIANGLE_STRIP );
  }

  void Renderer::draw( CameraPtr camera )
  {
    if ( !g_framebuf->available() )
      return;

    g_framebuf->begin();
    sceneDraw( camera );
    g_framebuf->end();

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    shaders_->use( 1 );

    glDisable( GL_DEPTH_TEST );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, g_framebuf->texture()->handle() );
    meshes_->getVAO( 1 ).draw( GL_TRIANGLES );
  }

  Renderer::~Renderer()
  {
    g_texture.reset();
    g_framebuf.reset();

    meshes_->teardown();

    shaders_->shutdown();
    shaders_.reset();
  }

}