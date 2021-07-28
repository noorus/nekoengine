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

  template <typename T>
  class PersistentBuffer {
  protected:
    GLuint id_;
    size_t size_;
    span<T> buffer_;
  public:
    PersistentBuffer( size_t count = 1 ): id_( 0 )
    {
      assert( count > 0 );
      size_ = ( count * sizeof( T ) );
      gl::glCreateBuffers( 1, &id_ );
      auto mapflags = ( gl::GL_MAP_WRITE_BIT | gl::GL_MAP_READ_BIT | gl::GL_MAP_PERSISTENT_BIT | gl::GL_MAP_COHERENT_BIT );
      auto storeflags = ( mapflags | gl::GL_DYNAMIC_STORAGE_BIT );
      gl::glNamedBufferStorage( id_, size_, nullptr, storeflags );
      auto buf = reinterpret_cast<T*>( gl::glMapNamedBufferRange( id_, 0, size_, mapflags ) );
      buffer_ = span<T>( buf, size_ );
    }
    inline GLuint id() const { return id_; }
    inline span<T>& buffer() { return buffer_; }
    ~PersistentBuffer()
    {
      gl::glUnmapNamedBuffer( id_ );
      gl::glDeleteBuffers( 1, &id_ );
    }
  };

  template <typename T>
  class SmarterBuffer {
  protected:
    GLuint id_;
    size_t size_;
    span<T> buffer_;
  public:
    SmarterBuffer( size_t count = 1 ): id_( 0 )
    {
      assert( count > 0 );
      size_ = ( count * sizeof( T ) );
      gl::glCreateBuffers( 1, &id_ );
      /*auto mapflags = ( gl::GL_MAP_WRITE_BIT | gl::GL_MAP_READ_BIT | gl::GL_MAP_PERSISTENT_BIT | gl::GL_MAP_COHERENT_BIT );
      auto storeflags = ( mapflags | gl::GL_DYNAMIC_STORAGE_BIT );
      gl::glNamedBufferStorage( id_, size_, nullptr, storeflags );*/
      gl::glNamedBufferData( id_, size_, nullptr, gl::GL_STREAM_DRAW );
    }
    inline void lock()
    {
      gl::glNamedBufferData( id_, size_, nullptr, gl::GL_STREAM_DRAW );
      auto buf = reinterpret_cast<T*>( gl::glMapNamedBuffer( id_, gl::GL_WRITE_ONLY ) );
      buffer_ = span<T>( buf, size_ );
    }
    inline void unlock()
    {
      gl::glUnmapNamedBuffer( id_ );
    }
    inline GLuint id() const { return id_; }
    inline span<T>& buffer() { return buffer_; }
    ~SmarterBuffer()
    {
      // FIXME - Access violation. what the fuck?
      // gl::glDeleteBuffers( 1, &id_ );
    }
  };

  class AttribWriter {
  private:
    struct Record {
      GLenum type_;
      GLsizei count_;
      size_t size_;
      bool normalize_;
      Record( GLenum type, GLsizei count, size_t size, bool normalize = false ):
      type_( type ), count_( count ), size_( size ), normalize_( normalize )
      {
      }
    };
    vector<Record> recs_;
    GLsizei stride_;
  public:
    AttribWriter(): stride_( 0 ) {}
    inline GLsizei stride() const { return stride_; }
    void add( GLenum type, GLsizei count, bool normalize = false )
    {
      GLsizei size = 0;
      if ( type == gl::GL_FLOAT )
        size = ( count * sizeof( float ) );
      else if ( type == gl::GL_UNSIGNED_BYTE )
        size = ( count * sizeof( uint8_t ) );
      else
        NEKO_EXCEPT( "Unsupported vertex attribute type in writer" );

      recs_.emplace_back( type, count, size, normalize );
      stride_ += size;
    }
    void write( GLuint handle )
    {
      GLuint ptr = 0;
      for ( GLuint i = 0; i < recs_.size(); ++i )
      {
        gl::glEnableVertexArrayAttrib( handle, i );
        gl::glVertexArrayAttribBinding( handle, i, 0 );
        gl::glVertexArrayAttribFormat( handle, i, recs_[i].count_, recs_[i].type_, recs_[i].normalize_ ? gl::GL_TRUE : gl::GL_FALSE, ptr );
        ptr += (GLuint)recs_[i].size_;
      }
    }
  };

  enum VBOType
  {
    VBO_2D, //!< 0: pos[2], 1: texcoord[2]
    VBO_3D, //!< 0: pos[3], 1: normal[3], 2: texcoord[2], 3: color, 4: tangent, 5: bitangent
    VBO_MyGUI, //!< 0: pos[3], 1: color[4], 1: texcoord[2]
    Max_VBOType
  };

  enum MeshAttributeIndex: GLuint
  {
    MeshAttrib_Position = 0,
    MeshAttrib_Texcoord,
    MeshAttrib_Color
  };

  enum MeshDataModifyHint
  {
    ModifyHint_Ephemeral,
    ModifyHint_Never,
    ModifyHint_Often
  };

  //! \class VBO
  //! \brief Vertex Buffer Object
  //! A buffer of raw vertex data (of only one type), referenceable by an ID.
  class VBO {
  private:
    std::variant<
      vector<Vertex2D>,
      vector<Vertex3D>,
      vector<MyGUI::Vertex>
    > store_;
  public:
    VBOType type_; //!< Vertex format
    GLuint id_; //!< GL name for this vbo
    bool uploaded_; //!< Has this been uploaded yet?
    bool dirty_; //!< Whether to just sub-update data
    bool mappable_;
    MeshDataModifyHint hint_; //!< What usage hint to use

    explicit VBO( const VBOType type, bool mappable ):
    type_( type ), id_( 0 ), uploaded_( false ), dirty_( false ), hint_( MeshDataModifyHint::ModifyHint_Often ), mappable_( mappable )
    {
      switch ( type_ )
      {
        case VBO_2D: store_ = vector<Vertex2D>(); break;
        case VBO_3D: store_ = vector<Vertex3D>(); break;
        case VBO_MyGUI: store_ = vector<MyGUI::Vertex>(); break;
        default: NEKO_EXCEPT( "Bad VBO type" ); break;
      }
    }

    VBO& operator=( VBO& other )
    {
      type_ = other.type_;
      id_ = other.id_;
      uploaded_ = other.uploaded_;
      dirty_ = other.dirty_;
      hint_ = other.hint_;
      store_ = move( other.store_ );
    }

    inline void copy( VBO& other )
    {
      type_ = other.type_;
      id_ = other.id_;
      uploaded_ = other.uploaded_;
      dirty_ = other.dirty_;
      hint_ = other.hint_;
      if ( type_ == VBO_2D )
        pushVertices( other.v2d() );
      else if ( type_ == VBO_3D )
        pushVertices( other.v3d() );
      else if ( type_ == VBO_MyGUI )
        pushVertices( other.guiv() );
    }

    inline uint64_t sizeInBytes() const
    {
      const auto typeSize = ( type_ == VBO_2D ? sizeof( Vertex2D )
        : type_ == VBO_3D ? sizeof( Vertex3D )
        : sizeof( MyGUI::Vertex ) );
      return ( sizeof( VBO ) + size() * typeSize );
    }

    inline vector<Vertex2D>& v2d()
    {
      assert( type_ == VBO_2D );
      return std::get<vector<Vertex2D>>( store_ );
    }
    inline vector<Vertex3D>& v3d()
    {
      assert( type_ == VBO_3D );
      return std::get<vector<Vertex3D>>( store_ );
    }
    inline vector<MyGUI::Vertex>& guiv()
    {
      assert( type_ == VBO_MyGUI );
      return std::get<vector<MyGUI::Vertex>>( store_ );
    }

    inline const bool empty() const
    {
      return ( type_ == VBO_2D ? std::get<vector<Vertex2D>>( store_ ).empty()
        : type_ == VBO_3D ? std::get<vector<Vertex3D>>( store_ ).empty()
        : std::get<vector<MyGUI::Vertex>>( store_ ).empty() );
    }

    inline const bool size() const
    {
      return ( type_ == VBO_2D ? std::get<vector<Vertex2D>>( store_ ).size()
        : type_ == VBO_3D ? std::get<vector<Vertex3D>>( store_ ).size()
        : std::get<vector<MyGUI::Vertex>>( store_ ).size() );
    }

    inline void pushVertices( const vector<Vertex2D>& vertices )
    {
      assert( type_ == VBO_2D );
      auto& store = std::get<vector<Vertex2D>>( store_ );
      store.insert( store.end(), vertices.begin(), vertices.end() );
      dirty_ = true;
    }

    inline void pushVertices( const vector<Vertex3D>& vertices )
    {
      assert( type_ == VBO_3D );
      auto& store = std::get<vector<Vertex3D>>( store_ );
      store.insert( store.end(), vertices.begin(), vertices.end() );
      dirty_ = true;
    }

    inline void pushVertices( const vector<MyGUI::Vertex>& vertices )
    {
      assert( type_ == VBO_MyGUI );
      auto& store = std::get<vector<MyGUI::Vertex>>( store_ );
      store.insert( store.end(), vertices.begin(), vertices.end() );
      dirty_ = true;
    }

    template <typename T>
    inline T* lock()
    {
      auto& store = std::get<vector<T>>( store_ );
      return (T*)store.data();
    }

    void unlock()
    {
      dirty_ = true;
    }

    inline void resize( size_t newSize )
    {
      if ( type_ == VBO_2D )
        v2d().resize( newSize );
      else if ( type_ == VBO_3D )
        v3d().resize( newSize );
      else if ( type_ == VBO_MyGUI )
        guiv().resize( newSize );
      dirty_ = true;
    }

    inline const size_t vertexCount() const
    {
      return ( type_ == VBO_2D ? std::get<vector<Vertex2D>>( store_ ).size()
        : type_ == VBO_3D ? std::get<vector<Vertex3D>>( store_ ).size()
        : std::get<vector<MyGUI::Vertex>>( store_ ).size() );
    }
  };

  using VBOPtr = shared_ptr<VBO>;

  //! \class EBO
  //! \brief Element Buffer Object
  //! An array of indices into a VBO.
  class EBO {
  public:
    vector<GLuint> storage_; //!< Contents of this ebo
    GLuint id_; //!< GL name for this ebo
    bool uploaded_; //!< Has this been uploaded yet?
    bool dirty_; //!< Whether to just sub-update data
    MeshDataModifyHint hint_; //!< What usage hint to use

    EBO(): id_( 0 ), uploaded_( false ), dirty_( false ), hint_( ModifyHint_Often ) {}

    EBO& operator=( EBO& other )
    {
      id_ = other.id_;
      uploaded_ = other.uploaded_;
      dirty_ = other.dirty_;
      hint_ = other.hint_;
      storage_ = move( other.storage_ );
    }

    inline void copy( const EBO& other )
    {
      id_ = other.id_;
      uploaded_ = other.uploaded_;
      dirty_ = other.dirty_;
      hint_ = other.hint_;
      storage_ = other.storage_;
    }

    inline const bool empty() const { return storage_.empty(); }

    inline uint64_t sizeInBytes() const
    {
      return ( sizeof( EBO ) + storage_.size() * sizeof( GLuint ) );
    }
  };

  using EBOPtr = shared_ptr<EBO>;

  //! \class VAO
  //! \brief Vertex Array Object
  //! An object that describes structure of variables in a VBO, referenceable by an ID.
  class VAO {
  public:
    VBOPtr vbo_; //!< Associated vbo
    EBOPtr ebo_; //!< Associated ebo, optional
    GLuint id_; //< GL name for this vao
    bool uploaded_; //!< Has this been uploaded yet?

    inline void reset()
    {
      vbo_.reset();
      ebo_.reset();
      id_ = 0;
      uploaded_ = false;
    }

    void begin();
    void draw( GLenum mode, int count = 0 );
    bool valid() const
    {
      if ( !vbo_ || !vbo_->uploaded_ || vbo_->dirty_ )
        return false;
      if ( ebo_ && ( !ebo_->uploaded_ || ebo_->dirty_ ) )
        return false;
      return true;
    }
    VAO(): id_( 0 ), uploaded_( false ) {}
  };

  using VAOPtr = shared_ptr<VAO>;

  class DynamicMesh {
  public:
    MeshManagerPtr manager_;
    VBOPtr vbo_;
    EBOPtr ebo_;
    VAOPtr vao_;
    GLenum drawMode_;
    struct Flags {
      bool indices : 1;
      bool mappable_ : 1;
    } flags_;
  public:
    DynamicMesh( MeshManagerPtr manager, VBOType vertexType, GLenum drawMode, bool useIndices, bool mappable );
    ~DynamicMesh();
    void pushVertices( const vector<Vertex2D>& vertices );
    void pushVertices( const vector<Vertex3D>& vertices );
    void pushVertices( const vector<MyGUI::Vertex>& vertices );
    void pushIndices( const vector<GLuint>& indices );
    inline const size_t vertsCount() const
    {
      assert( vbo_ );
      return vbo_->vertexCount();
    }
    inline const size_t indicesCount() const
    {
      assert( ebo_ );
      return ebo_->storage_.size();
    }
    void begin();
    void draw( int count = 0 );
    template <typename T>
    inline void append( const vector<T>& vertices, vector<GLuint> indices )
    {
      assert( vbo_ && ebo_ );
      auto base = (GLuint)vertsCount();
      pushVertices( vertices );
      std::for_each( indices.begin(), indices.end(), [base]( GLuint& n ) { n += base; } );
      pushIndices( indices );
    }
    void resize( size_t newSize );
  };

  using DynamicMeshPtr = shared_ptr<DynamicMesh>;
  using DynamicMeshVector = vector<DynamicMeshPtr>;

  class JSMesh {
  public:
    size_t id_;
    VBOPtr vbo_;
    EBOPtr ebo_;
    VAOPtr vao_;
    /*void copyFrom( const JSMesh& other )
    {
      localVBO_.type_ = other.localVBO_.type_;
      localVBO_.
    }*/
    JSMesh();
  };

  class StaticMesh {
  private:
    MeshManagerPtr manager_;
    VBOPtr vbo_;
    EBOPtr ebo_;
    VAOPtr vao_;
    GLenum drawMode_;
    GLsizei size_;
  public:
    StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex2D> verts );
    StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex2D> verts, vector<GLuint> indices );
    StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex3D> verts );
    StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex3D> verts, vector<GLuint> indices );
    ~StaticMesh();
    void begin();
    void draw();
  };

  using StaticMeshPtr = shared_ptr<StaticMesh>;
  using StaticMeshVector = vector<StaticMeshPtr>;

  class MeshGenerator {
  public:
    pair<vector<Vertex3D>, vector<GLuint>> makePlane( vec2 dimensions, vec2u segments, vec3 normal );
  };

}