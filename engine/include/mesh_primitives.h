#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "utilities.h"
#include "neko_pooledtypes.h"

#ifndef NEKO_NO_GUI
#include <MyGUI/MyGUI_VertexData.h>
#else
namespace MyGUI {
  struct Vertex {
    int dummy = 0;
  };
}
#endif

namespace neko {

  namespace util {

    void generateTangentsAndBitangents( vector<Vertex3D>& verts, const vector<GLuint>& indices );

  }

  struct Pipeline;

  template <typename T>
  class MappedGLBuffer {
    static constexpr glbinding::SharedBitfield<gl::BufferStorageMask, gl::MapBufferAccessMask> c_glMapFlags = ( gl::GL_MAP_WRITE_BIT | gl::GL_MAP_READ_BIT | gl::GL_MAP_PERSISTENT_BIT | gl::GL_MAP_COHERENT_BIT );
    static constexpr glbinding::SharedBitfield<gl::BufferStorageMask, gl::MapBufferAccessMask> c_glStoreFlags = ( c_glMapFlags | gl::GL_DYNAMIC_STORAGE_BIT );
  protected:
    GLuint id_ = 0;
    size_t count_;
    bool mapped_ = false;
  public:
    MappedGLBuffer( size_t count = 1 ): count_( count )
    {
      assert( count > 0 );
      gl::glCreateBuffers( 1, &id_ );
      gl::glNamedBufferStorage( id_, count_ * sizeof( T ), nullptr, c_glStoreFlags );
    }
    inline GLuint id() const { return id_; }
    inline size_t size() const noexcept { return count_; }
    inline size_t bytesize() const noexcept { return ( count_ * sizeof( T ) ); }
    inline span<T> lock( gl::GLintptr offset = 0, gl::GLint count = 0 )
    {
      if ( count < 1 )
        count = static_cast<GLint>( count_ );
      auto buf = reinterpret_cast<T*>( gl::glMapNamedBufferRange( id_, offset, count * sizeof( T ), c_glMapFlags ) );
      mapped_ = true;
      return span<T>( buf, count );
    }
    inline void unlock()
    {
      auto ret = gl::glUnmapNamedBuffer( id_ );
      assert( ret == gl::GL_TRUE );
      mapped_ = false;
    }
    ~MappedGLBuffer()
    {
      assert( !mapped_ );
      gl::glDeleteBuffers( 1, &id_ );
    }
  };

  enum AttributeType
  {
    Attrib_Pos2D = 0,
    Attrib_Pos3D,
    Attrib_Texcoord2D,
    Attrib_Normal3D,
    Attrib_Color4D,
    Attrib_OrientationQuat,
    Attrib_Scale3D,
    Attrib_Tangent3D,
    Attrib_Tangent4D,
    Attrib_Bitangent3D
  };

  class AttribWriter: public nocopy {
  private:
    struct Record {
      AttributeType type_;
      GLenum stype_;
      GLsizei count_;
      size_t size_;
      bool normalize_;
      Record( AttributeType type, GLenum stype, GLsizei count, size_t size, bool normalize = false ):
      type_( type ), stype_( stype ), count_( count ), size_( size ), normalize_( normalize )
      {
      }
    };
    vector<Record> recs_;
    GLsizei stride_;
  public:
    AttribWriter(): stride_( 0 ) {}
    inline GLsizei stride() const { return stride_; }
    void add( AttributeType type, GLenum datatype = gl::GL_FLOAT, bool normalize = false )
    {
      GLsizei count = 0;
      if ( type == Attrib_Pos2D )
      {
        count = 2;
      }
      else if ( type == Attrib_Pos3D )
      {
        count = 3;
      }
      else if ( type == Attrib_Texcoord2D )
      {
        count = 2;
      }
      else if ( type == Attrib_Normal3D )
      {
        count = 3;
      }
      else if ( type == Attrib_Color4D )
      {
        count = 4;
      }
      else if ( type == Attrib_OrientationQuat )
      {
        count = 4;
      }
      else if ( type == Attrib_Scale3D )
      {
        count = 3;
      }
      else if ( type == Attrib_Tangent3D )
      {
        count = 3;
      }
      else if ( type == Attrib_Tangent4D )
      {
        count = 4;
      }
      else if ( type == Attrib_Bitangent3D )
      {
        count = 3;
      }
      else
        NEKO_EXCEPT( "Unknown vertex attribute type supplied to writer" );
      GLsizei size = 0;
      if ( datatype == gl::GL_FLOAT )
        size = ( count * sizeof( float ) );
      else if ( datatype == gl::GL_UNSIGNED_BYTE )
        size = ( count * sizeof( uint8_t ) );
      else
        NEKO_EXCEPT( "Unsupported vertex attribute type in writer" );
      recs_.emplace_back( type, datatype, count, size, normalize );
      stride_ += size;
    }
    void write( GLuint handle )
    {
      GLuint ptr = 0;
      for ( GLuint i = 0; i < recs_.size(); ++i )
      {
        gl::glEnableVertexArrayAttrib( handle, i );
        gl::glVertexArrayAttribBinding( handle, i, 0 );
        gl::glVertexArrayAttribFormat( handle, i, recs_[i].count_, recs_[i].stype_, recs_[i].normalize_ ? gl::GL_TRUE : gl::GL_FALSE, ptr );
        ptr += (GLuint)recs_[i].size_;
      }
    }
  };

  template <typename VertexType>
  class KnownVertexAttributes: public AttribWriter {
  public:
    KnownVertexAttributes(): AttribWriter()
    {
      if constexpr ( std::is_same_v<VertexType, Vertex2D> )
      {
        add( Attrib_Pos2D ); // vec2 position
        add( Attrib_Texcoord2D ); // vec2 texcoord
      }
      else if constexpr ( std::is_same_v<VertexType, Vertex3D> )
      {
        add( Attrib_Pos3D ); // vec3 position
        add( Attrib_Normal3D ); // vec3 normal
        add( Attrib_Texcoord2D ); // vec2 texcoord
        add( Attrib_Color4D ); // vec4 color
        add( Attrib_Tangent4D ); // vec4 tangent
        add( Attrib_Bitangent3D ); // vec3 bitangent
      }
      else if constexpr ( std::is_same_v<VertexType, VertexText> )
      {
        add( Attrib_Pos3D ); // vec3 position
        add( Attrib_Texcoord2D ); // vec2 texcoord
        add( Attrib_Color4D ); // vec4 color
      }
      else
      {
        static_assert( false, "Unsupported vertex type" );
      }
    }
  };

  class MeshGenerator {
  public:
    pair<vector<Vertex3D>, vector<GLuint>> makePlane(
      vec2 dimensions, vec2u segments, vec3 normal, vec4 color = vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
    pair<vector<Vertex3D>, vector<GLuint>> makeBox(
      vec3 dimensions, vec2u segments, bool inverted = false, vec4 color = vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
    pair<vector<Vertex3D>, vector<GLuint>> makeSphere(
      Real diameter, vec2u segments, vec4 color = vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
    pair<vector<Vertex3D>, vector<GLuint>> makeTerrain( vec2i size, vec3 dimensions );
  };

}