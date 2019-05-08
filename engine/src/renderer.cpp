#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"
#include "meshmanager.h"
#include "shaders.h"
#include "camera.h"

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

  Renderer::Renderer( EnginePtr engine ): engine_( move( engine ) )
  {
    shaders_ = make_shared<Shaders>( engine_ );
    shaders_->initialize();

    meshes_ = make_shared<MeshManager>();
    auto quadVBO = meshes_->pushVBO( static_geometry::quad2D );
    meshes_->uploadVBOs();
    auto triangleVao = meshes_->pushVAO( VAO::VBO_2D, quadVBO );
    meshes_->uploadVAOs();

    g_texture = make_shared<Texture>( this, 2, 2, GL_RGBA8, (const void*)static_geometry::image4x4.data() );
  }

  GLuint Renderer::implCreateTexture( size_t width, size_t height, GLGraphicsFormat format, const void* data )
  {
    GLuint handle;
    glGenTextures( 1, &handle );
    assert( handle != 0 );

    glBindTexture( GL_TEXTURE_2D, handle );

    // assumptions for now
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    // assumptions for now
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // glGenerateMipmap( GL_TEXTURE_2D );

    // 1 byte alignment
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glTexImage2D( GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

    glBindTexture( GL_TEXTURE_2D, 0 );

    return handle;
  }

  void Renderer::implDeleteTexture( GLuint handle )
  {
    assert( handle );
    glDeleteTextures( 1, &handle );
  }

  GLuint Renderer::implCreateRenderbuffer( size_t width, size_t height, GLGraphicsFormat format )
  {
    assert( width <= (size_t)GL_MAX_RENDERBUFFER_SIZE && height <= (size_t)GL_MAX_RENDERBUFFER_SIZE );

    GLuint handle;
    glGenRenderbuffers( 1, &handle );
    assert( handle != 0 );

    glBindRenderbuffer( GL_RENDERBUFFER, handle );
    glRenderbufferStorage( GL_RENDERBUFFER, format, (GLsizei)width, (GLsizei)height );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );

    return handle;
  }

  void Renderer::implDeleteRenderbuffer( GLuint handle )
  {
    assert( handle );
    glDeleteRenderbuffers( 1, &handle );
  }

  void Renderer::draw( CameraPtr camera )
  {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDisable( GL_DEPTH_TEST );
    glDepthMask( 0 );

    mat4 model( 1.0f );
    model = glm::scale( model, vec3( 100.0f, 100.0f, 1.0f ) );
    shaders_->setMatrices( model, camera->view(), camera->projection() );

    shaders_->use( 0 );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, g_texture->handle() );

    meshes_->getVAO( 0 ).draw( GL_TRIANGLE_STRIP );
  }

  Renderer::~Renderer()
  {
    g_texture.reset();

    meshes_->teardown();

    shaders_->shutdown();
    shaders_.reset();
  }

}