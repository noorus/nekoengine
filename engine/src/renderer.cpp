#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"
#include "meshmanager.h"
#include "shaders.h"
#include "camera.h"
#include "loader.h"
#include "engine.h"
#include "fontmanager.h"

namespace neko {

  using namespace gl;

  namespace BuiltinData {

    const vector<PixelRGBA> placeholderImage2x2 =
    {
      { 255, 0,   0,   255 },
      { 0,   255, 0,   255 },
      { 0,   0,   255, 255 },
      { 255, 0,   255, 255 }
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

    const vector<GLuint> quadIndices =
    {
      0, 1, 2, 3
    };

  }

  const int64_t c_glVersion[2] = { 4, 5 };

  static TexturePtr g_texture;
  static FramebufferPtr g_framebuf;

  void glStartupFetchAndCheck( GLInformation& info )
  {
    auto glbAuxStr = []( GLenum e ) -> utf8String
    {
      return glbinding::aux::Meta::getString( e );
    };
    auto glvGetI32NoThrow = []( GLenum e  ) -> int32_t
    {
      GLint data = 0;
      glGetIntegerv( e, &data );
      return data;
    };
    auto glvGetI64 = [glbAuxStr]( GLenum e, int64_t& target, bool doNotThrow = false ) -> bool
    {
      GLint64 data = 0;
      glGetInteger64v( e, &data );
      auto error = glGetError();
      if ( error != GL_NO_ERROR )
      {
        if ( !doNotThrow )
          NEKO_OPENGL_EXCEPT( "glGetInteger64v failed for " + glbAuxStr( e ), error );
        return false;
      }
      target = (int64_t)data;
      return true;
    };

    // OpenGL version
    info.versionMajor = glvGetI32NoThrow( GL_MAJOR_VERSION );
    info.versionMinor = glvGetI32NoThrow( GL_MINOR_VERSION );

    // Bail out if the version isn't up to task.
    // The following (newer) calls might otherwise fail in a less controlled way.
    // For example, glGetInteger64v already requires OpenGL 3.2
    if ( info.versionMajor < c_glVersion[0] || ( info.versionMajor == c_glVersion[0] && info.versionMinor < c_glVersion[1] ) )
    {
      // Be all pretty and informative about it.
      std::stringstream description;
      description << "Insufficient OpenGL version!\r\nGot: ";
      if ( info.versionMajor == 0 )
        description << "ancient";
      else
        description << info.versionMajor << "." << info.versionMinor;
      description << "; Expected at least " << c_glVersion[0] << "." << c_glVersion[1];
      NEKO_EXCEPT( description.str() );
    }

    // Max buffer sizes
    glvGetI64( GL_MAX_TEXTURE_SIZE, info.maxTextureSize );
    glvGetI64( GL_MAX_RENDERBUFFER_SIZE, info.maxRenderbufferSize );

    if ( !glvGetI64( GL_MAX_FRAMEBUFFER_WIDTH, info.maxFramebufferWidth, true )
      || !glvGetI64( GL_MAX_FRAMEBUFFER_HEIGHT, info.maxFramebufferHeight, true ) )
    {
      // Some drivers do not support GL_MAX_FRAMEBUFFER_WIDTH/GL_MAX_FRAMEBUFFER_WIDTH
      // despite supporting framebuffers in themselves just fine, due to them being added
      // in a later extension than framebuffers. Let's just default to the max texture size.
      info.maxFramebufferWidth = info.maxTextureSize;
      info.maxFramebufferHeight = info.maxTextureSize;
    }
  }

  void Renderer::clearErrors()
  {
    uint32_t errorCount = 0;
    while ( true )
    {
      auto error = glGetError();
      if ( error == GL_NO_ERROR )
        break;
      if ( error == GL_OUT_OF_MEMORY )
        NEKO_EXCEPT( "Driver generated GL_OUT_OF_MEMORY. Out of memory." );
      errorCount++;
    }
    if ( !errorCount )
      return;
    engine_->console()->printf( Console::srcGfx, "Cleared %d GL errors...", errorCount );
  }

  Renderer::Renderer( EnginePtr engine ): engine_( move( engine ) )
  {
    clearErrors();
    glStartupFetchAndCheck( info_ );
  }

  DynamicMeshPtr g_testMesh;

  void Renderer::preInitialize()
  {
    clearErrors();

    shaders_ = make_shared<Shaders>( engine_ );
    shaders_->initialize();

    meshes_ = make_shared<MeshManager>();

    builtin_.placeholderTexture_ = createTextureWithData( 2, 2, PixFmtColorRGBA8,
      (const void*)BuiltinData::placeholderImage2x2.data() );
    builtin_.screenQuad_ = meshes_->createStatic( GL_TRIANGLES, BuiltinData::screenQuad );

    //g_texture = make_shared<Texture>( this, 8192, 8192, PixFmtColorR8, engine_->fonts()->fonts()[0]->atlas_->data_.data() );
  }

  void Renderer::initialize( size_t width, size_t height )
  {
    MaterialPtr myMat = make_shared<Material>();
    materials_.push_back( myMat );
    engine_->loader()->addLoadTask( { LoadTask( myMat, R"(data\textures\test.png)" ) } );

    g_framebuf = make_shared<Framebuffer>( this );

    reset( width, height );
  }

  void Renderer::reset( size_t width, size_t height )
  {
    assert( g_framebuf );
    g_framebuf->recreate( width, height );
  }

  void Renderer::uploadTextures()
  {
    MaterialVector mats;
    engine_->loader()->getFinishedMaterials( mats );
    if ( !mats.empty() )
      engine_->console()->printf( Console::srcGfx, "Renderer::uploadTextures got %d new materials", mats.size() );
    for ( auto& mat : mats )
    {
      if ( !mat->loaded_ )
        continue;
      mat->texture_ = make_shared<Texture>( this, mat->image_.width_, mat->image_.height_, mat->image_.format_, mat->image_.data_.data() );
    }
  }

  void Renderer::prepare( GameTime time )
  {
    // Upload any new textures. Could this be parallellized?
    uploadTextures();

    // VAOs can and will refer to VBOs and EBOs, and those must have been uploaded by the point when we try to create the VAO.
    // Thus uploading the VAOs should always come last.
    meshes_->uploadVBOs();
    meshes_->uploadEBOs();
    meshes_->uploadVAOs();

    if ( !g_testMesh && !materials_.empty() && materials_[0]->loaded_ )
    {
      g_testMesh = meshes_->createDynamic( GL_TRIANGLE_STRIP );
      g_testMesh->pushVertices( BuiltinData::quadStrip2D );
      g_testMesh->pushIndices( BuiltinData::quadIndices );
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

    // Assumptions for now
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    // Assumptions for now
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // 1 byte alignment - i.e. unaligned.
    // Could boost performance to use aligned memory in the future.
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

  MaterialPtr Renderer::createTextureWithData( size_t width, size_t height, PixelFormat format, const void* data )
  {
    MaterialPtr mat = make_shared<Material>();
    mat->image_.format_ = format;
    mat->image_.width_ = (unsigned int)width;
    mat->image_.height_ = (unsigned int)height;
    mat->texture_ = make_shared<Texture>( this, mat->image_.width_, mat->image_.height_, mat->image_.format_, data );
    mat->loaded_ = true;
    return move( mat );
  }

  void Renderer::sceneDraw( CameraPtr camera )
  {
    glDisable( GL_DEPTH_TEST );
    glDepthMask( 0 );

    mat4 model( 1.0f );
    model = glm::scale( model, vec3( 256.0f, 256.0f, 1.0f ) );
    model = glm::translate( model, vec3( 1.0f, 1.0f, 0.0f ) );
    shaders_->setMatrices( model, camera->view(), camera->projection() );

    if ( g_testMesh )
    {
      shaders_->use( 0 );

      glActiveTexture( GL_TEXTURE0 );
      if ( !materials_.empty() && materials_[0]->texture_ )
        glBindTexture( GL_TEXTURE_2D, materials_[0]->texture_->handle() );
      else
        glBindTexture( GL_TEXTURE_2D, g_texture->handle() );

      g_testMesh->draw();
    }
  }

  void Renderer::draw( CameraPtr camera )
  {
    if ( !g_framebuf->available() )
      return;

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );

    glEnable( GL_MULTISAMPLE );

    // Smoothing can generate sub-fragments and cause visible ridges between triangles.
    // Use a framebuffer for AA instead.
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_POLYGON_SMOOTH );

    // Draw the scene inside the framebuffer.
    g_framebuf->begin();
    sceneDraw( camera );
    g_framebuf->end();

    // Framebuffer has been unbound, now draw to the default context, the window.
    glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    shaders_->use( 1 );

    glDisable( GL_DEPTH_TEST );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, g_framebuf->texture()->handle() );
    builtin_.screenQuad_->draw();
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