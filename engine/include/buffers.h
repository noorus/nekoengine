#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "neko_platform.h"
#include "texture.h"
#include "framebuffer.h"
#include "renderbuffer.h"
#include "materials.h"
#include "meshmanager.h"
#include "modelmanager.h"
#include "scripting.h"
#include "shaders.h"
#include "math_aabb.h"

namespace neko {

  template <size_t Count>
  class PointRenderBuffer {
  protected:
    static constexpr GLuint c_maxVertices = Count;
    unique_ptr<MappedGLBuffer<VertexPointParticle>> buffer_;
    GLuint vao_ = 0;
  public:
    PointRenderBuffer()
    {
      buffer_ = make_unique<MappedGLBuffer<VertexPointParticle>>( c_maxVertices );
      gl::glCreateVertexArrays( 1, &vao_ );
      neko::AttribWriter attribs;
      attribs.add( gl::GL_FLOAT, 3 ); // vec3 position
      attribs.add( gl::GL_FLOAT, 4 ); // quat orientation
      attribs.add( gl::GL_FLOAT, 3 ); // vec3 size
      attribs.add( gl::GL_FLOAT, 4 ); // vec4 color
      attribs.write( vao_ );
      gl::glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs.stride() );
    }
    inline MappedGLBuffer<VertexPointParticle>& buffer() { return *buffer_.get(); }
    void draw( shaders::Pipeline& pipeline, GLsizei count, GLint base = 0, gl::GLenum mode = gl::GL_POINTS )
    {
      gl::glBindVertexArray( vao_ );
      mat4 mdl( 1.0f );
      pipeline.setUniform( "model", mdl );
      gl::glDrawArrays( mode, base, count );
      gl::glBindVertexArray( 0 );
    }
    ~PointRenderBuffer()
    {
      gl::glDeleteVertexArrays( 1, &vao_ );
      buffer_.reset();
    }
  };

  template <size_t Count>
  class LineRenderBuffer {
  protected:
    const GLuint c_maxVertices = Count;
    unique_ptr<MappedGLBuffer<VertexLine>> buffer_;
    GLuint vao_ = 0;
  public:
    LineRenderBuffer()
    {
      buffer_ = make_unique<MappedGLBuffer<VertexLine>>( c_maxVertices );
      gl::glCreateVertexArrays( 1, &vao_ );
      neko::AttribWriter attribs;
      attribs.add( gl::GL_FLOAT, 3 ); // vec3 position
      attribs.add( gl::GL_FLOAT, 4 ); // vec4 color
      attribs.write( vao_ );
      gl::glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs.stride() );
    }
    inline MappedGLBuffer<VertexLine>& buffer() { return *buffer_.get(); }
    void draw( shaders::Pipeline& pipeline, GLsizei count, GLint base = 0, gl::GLenum mode = gl::GL_POINTS )
    {
      gl::glBindVertexArray( vao_ );
      mat4 mdl( 1.0f );
      pipeline.setUniform( "model", mdl );
      gl::glDrawArrays( mode, base, count );
      gl::glBindVertexArray( 0 );
    }
    ~LineRenderBuffer()
    {
      gl::glDeleteVertexArrays( 1, &vao_ );
      buffer_.reset();
    }
  };

}