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

  struct VertexTypeBase
  {
  };

  template <typename VertexType>
  class VertexBufferBase {
  protected:
    using BufferType = MappedGLBuffer<VertexType>;
    unique_ptr<BufferType> buffer_;
    AttribWriter attribs_;
    GLuint vao_ = 0;
  public:
    VertexBufferBase( size_t maxVertices )
    {
      buffer_ = make_unique<BufferType>( maxVertices );
      gl::glCreateVertexArrays( 1, &vao_ );
      attribs_.add( Attrib_Pos3D ); // vec3 position
      attribs_.add( Attrib_OrientationQuat ); // quat orientation
      attribs_.add( Attrib_Scale3D ); // vec3 size
      attribs_.add( Attrib_Color4D ); // vec4 color
      attribs_.write( vao_ );
      gl::glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs_.stride() );
    }
    inline BufferType& buffer() { return *buffer_; }
    ~VertexBufferBase()
    {
      gl::glDeleteVertexArrays( 1, &vao_ );
      buffer_.reset();
    }
  };

  class BasicIndexedVertexbuffer {
  protected:
    using BufferType = MappedGLBuffer<Vertex3D>;
    using IndicesType = MappedGLBuffer<gl::GLuint>;
    unique_ptr<BufferType> buffer_;
    unique_ptr<IndicesType> indices_;
    AttribWriter attribs_;
    GLuint vao_ = 0;
  public:
    BasicIndexedVertexbuffer( size_t maxVertices, size_t maxIndices )
    {
      buffer_ = make_unique<BufferType>( maxVertices );
      indices_ = make_unique<IndicesType>( maxIndices );
      gl::glCreateVertexArrays( 1, &vao_ );
      gl::glVertexArrayElementBuffer( vao_, indices_->id() );
      attribs_.add( Attrib_Pos3D ); // vec3 position
      attribs_.add( Attrib_Normal3D ); // vec3 normal
      attribs_.add( Attrib_Texcoord2D ); // vec2 texcoord
      attribs_.add( Attrib_Color4D ); // vec4 color
      attribs_.add( Attrib_Tangent4D ); // vec4 tangent
      attribs_.add( Attrib_Bitangent3D ); // vec3 bitangent
      attribs_.write( vao_ );
      gl::glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs_.stride() );
    }
    inline BufferType& buffer() { return *buffer_; }
    inline IndicesType& indices() { return *indices_; }
    void draw( shaders::Shaders& shaders, const Material& mat, const mat4& model )
    {
      gl::glBindVertexArray( vao_ );

      gl::glEnable( gl::GL_CULL_FACE );
      gl::glCullFace( gl::GL_BACK );
      gl::glFrontFace( gl::GL_CCW );
      auto ppl = &shaders.usePipeline( "mat_unlit" );
      const auto hndl = mat.layers_[0].texture_->handle();
      gl::glBindTextures( 0, 1, &hndl );
      ppl->setUniform( "tex", 0 );
      ppl->setUniform( "model", model );
      gl::glDrawElements( gl::GL_TRIANGLES, static_cast<gl::GLsizei>( indices_->size() ), gl::GL_UNSIGNED_INT, nullptr );
      gl::glBindVertexArray( 0 );
    }
    ~BasicIndexedVertexbuffer()
    {
      gl::glDeleteVertexArrays( 1, &vao_ );
      indices_.reset();
      buffer_.reset();
    }
  };

  class SpriteVertexbuffer {
  protected:
    using BufferType = MappedGLBuffer<Vertex3D>;
    using IndicesType = MappedGLBuffer<gl::GLuint>;
    unique_ptr<BufferType> buffer_;
    unique_ptr<IndicesType> indices_;
    AttribWriter attribs_;
    GLuint vao_ = 0;
  public:
    SpriteVertexbuffer( size_t maxVertices, size_t maxIndices )
    {
      buffer_ = make_unique<BufferType>( maxVertices );
      indices_ = make_unique<IndicesType>( maxIndices );
      gl::glCreateVertexArrays( 1, &vao_ );
      gl::glVertexArrayElementBuffer( vao_, indices_->id() );
      attribs_.add( Attrib_Pos3D ); // vec3 position
      attribs_.add( Attrib_Normal3D ); // vec3 normal
      attribs_.add( Attrib_Texcoord2D ); // vec2 texcoord
      attribs_.add( Attrib_Color4D ); // vec4 color
      attribs_.add( Attrib_Tangent4D ); // vec4 tangent
      attribs_.add( Attrib_Bitangent3D ); // vec3 bitangent
      attribs_.write( vao_ );
      gl::glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs_.stride() );
    }
    inline BufferType& buffer() { return *buffer_; }
    inline IndicesType& indices() { return *indices_; }
    void draw( shaders::Shaders& shaders, const Material& mat, int frame, const mat4& model )
    {
      gl::glBindVertexArray( vao_ );
      auto ppl = &shaders.usePipeline( "sprite" );
      const auto hndl = mat.layers_[0].texture_->handle();
      gl::glBindTextures( 0, 1, &hndl );
      ppl->setUniform( "tex", 0 );
      ppl->setUniform( "tex_layer", frame < mat.arrayDepth_ ? frame : mat.arrayDepth_ - 1 );
      ppl->setUniform( "tex_dimensions", vec2( mat.width(), mat.height() ) );
      ppl->setUniform( "model", model );
      gl::glDrawElements( gl::GL_TRIANGLES, static_cast<gl::GLsizei>( indices_->size() ), gl::GL_UNSIGNED_INT, nullptr );
      gl::glBindVertexArray( 0 );
    }
    ~SpriteVertexbuffer()
    {
      gl::glDeleteVertexArrays( 1, &vao_ );
      indices_.reset();
      buffer_.reset();
    }
  };

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
      attribs.add( Attrib_Pos3D ); // vec3 position
      attribs.add( Attrib_OrientationQuat ); // quat orientation
      attribs.add( Attrib_Scale3D ); // vec3 size
      attribs.add( Attrib_Color4D ); // vec4 color
      attribs.write( vao_ );
      gl::glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs.stride() );
    }
    inline MappedGLBuffer<VertexPointParticle>& buffer() { return *buffer_; }
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
      attribs.add( Attrib_Pos3D ); // vec3 position
      attribs.add( Attrib_Color4D ); // vec4 color
      attribs.write( vao_ );
      gl::glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs.stride() );
    }
    inline MappedGLBuffer<VertexLine>& buffer() { return *buffer_; }
    void draw( shaders::Pipeline& pipeline, GLsizei count, GLint base = 0, gl::GLenum mode = gl::GL_POINTS )
    {
      gl::glBindVertexArray( vao_ );
      mat4 mdl( 1.0f );
      pipeline.setUniform( "model", mdl );
      gl::glDrawArrays( mode, base, count );
      gl::glBindVertexArray( 0 );
    }
    void draw( shaders::Pipeline& pipeline, const mat4& mdl, GLsizei count, GLint base, gl::GLenum mode )
    {
      gl::glBindVertexArray( vao_ );
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

#pragma pack( push, 1 )
  struct Vertex
  {
    vec3 position;
    vec2 texcoord;
    vec4 color;
    Vertex( vec3 pos, vec2 tc, vec4 clr ): position( pos ), texcoord( tc ), color( clr ) {}
  };
#pragma pack( pop )

  using VertexIndex = unsigned int;

  using Vertices = vector<Vertex>;
  using Indices = vector<VertexIndex>;

  class TextRenderBuffer {
  protected:
    using BufferType = MappedGLBuffer<Vertex>;
    using IndicesType = MappedGLBuffer<gl::GLuint>;
    unique_ptr<BufferType> buffer_;
    unique_ptr<IndicesType> indices_;
    GLuint vao_ = 0;
  public:
    TextRenderBuffer( GLuint maxVertices, GLuint maxIndices )
    {
      buffer_ = make_unique<BufferType>( maxVertices );
      indices_ = make_unique<IndicesType>( maxIndices );
      gl::glCreateVertexArrays( 1, &vao_ );
      gl::glVertexArrayElementBuffer( vao_, indices_->id() );
      neko::AttribWriter attribs;
      attribs.add( Attrib_Pos3D ); // vec3 position
      attribs.add( Attrib_Texcoord2D ); // vec2 texcoord
      attribs.add( Attrib_Color4D ); // vec4 color
      attribs.write( vao_ );
      gl::glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs.stride() );
    }
    inline BufferType& buffer() { return *buffer_; }
    inline IndicesType& indices() { return *indices_; }
    void draw( shaders::Shaders& shaders, const mat4& model, gl::GLuint texture )
    {
      gl::glBindVertexArray( vao_ );
      auto& pipeline = shaders.usePipeline( "text2d" );
      pipeline.setUniform( "model", model );
      pipeline.setUniform( "tex", 0 );
      gl::glBindTextureUnit( 0, texture );
      gl::glDrawElements( gl::GL_TRIANGLES, static_cast<gl::GLsizei>( indices_->size() ), gl::GL_UNSIGNED_INT, nullptr );
      gl::glBindVertexArray( 0 );
    }
    ~TextRenderBuffer()
    {
      gl::glDeleteVertexArrays( 1, &vao_ );
      indices_.reset();
      buffer_.reset();
    }
  };

}