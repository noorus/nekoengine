#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "utilities.h"
#include "neko_pooledtypes.h"

namespace neko {

  enum VBOType {
    VBO_3D, //!< 0: pos[3], 1: texcoord[2]
    VBO_2D, //!< 0: pos[2], 1: texcoord[2]
    VBO_Text, //!< 0: pos[3], 1: texcoord[2], 2: color[4]
    Max_VBOType
  };

  enum MeshAttributeIndex: GLuint {
    MeshAttrib_Position = 0,
    MeshAttrib_Texcoord,
    MeshAttrib_Color
  };

  enum MeshDataModifyHint {
    ModifyHint_Ephemeral,
    ModifyHint_Never,
    ModifyHint_Often
  };

  //! \class VBO
  //! \brief Vertex Buffer Object
  //! A buffer of raw vertex data (of only one type), referenceable by an ID.
  class VBO {
  private:
    std::variant<vector<Vertex2D>, vector<Vertex3D>, vector<VertexText3D>> store_;
  public:
    VBOType type_; //!< Vertex format
    GLuint id_; //!< GL name for this vbo
    bool uploaded_; //!< Has this been uploaded yet?
    bool dirty_; //!< Whether to just sub-update data
    MeshDataModifyHint hint_; //!< What usage hint to use

    explicit VBO( const VBOType type ):
      type_( type ), id_( 0 ), uploaded_( false ), dirty_( false ), hint_( MeshDataModifyHint::ModifyHint_Often )
    {
      switch ( type_ )
      {
        case VBO_2D: store_ = vector<Vertex2D>(); break;
        case VBO_3D: store_ = vector<Vertex3D>(); break;
        case VBO_Text: store_ = vector<VertexText3D>(); break;
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

    inline void copy( const VBO& other )
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
      else if ( type_ == VBO_Text )
        pushVertices( other.vt3d() );
    }

    inline const vector<Vertex2D>& v2d() const
    {
      assert( type_ == VBO_2D );
      return std::get<vector<Vertex2D>>( store_ );
    }
    inline const vector<Vertex3D>& v3d() const
    {
      assert( type_ == VBO_3D );
      return std::get<vector<Vertex3D>>( store_ );
    }
    inline const vector<VertexText3D>& vt3d() const
    {
      assert( type_ == VBO_Text );
      return std::get<vector<VertexText3D>>( store_ );
    }

    inline const bool empty() const
    {
      return ( type_ == VBO_2D ? std::get<vector<Vertex2D>>( store_ ).empty()
        : type_ == VBO_3D ? std::get<vector<Vertex3D>>( store_ ).empty()
        : std::get<vector<VertexText3D>>( store_ ).empty() );
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

    inline void pushVertices( const vector<VertexText3D>& vertices )
    {
      assert( type_ == VBO_Text );
      auto& store = std::get<vector<VertexText3D>>( store_ );
      store.insert( store.end(), vertices.begin(), vertices.end() );
      dirty_ = true;
    }

    inline const size_t vertexCount() const
    {
      return ( type_ == VBO_2D ? std::get<vector<Vertex2D>>( store_ ).size()
        : type_ == VBO_3D ? std::get<vector<Vertex3D>>( store_ ).size()
        : std::get<vector<VertexText3D>>( store_ ).size() );
    }
  };

  using VBOPtr = shared_ptr<VBO>;

  //! \class EBO
  //! \brief Element Buffer Object
  //! An array of indices into a VBO.
  class EBO {
  public:
    vector<GLuint> storage_; //!< Contents of this ebo
    GLuint id_; //!< GL"name for this ebo
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
    void draw( GLenum mode, GLsizei size );
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

  struct BareVAO {
    GLuint id_;
    bool uploaded_;
    BareVAO(): id_( 0 ), uploaded_( false ) {}
  };

  class DynamicMesh {
  public:
    MeshManagerPtr manager_;
    VBOPtr vbo_;
    EBOPtr ebo_;
    VAOPtr vao_;
    GLenum drawMode_;
  public:
    DynamicMesh( MeshManagerPtr manager, VBOType vertexType, GLenum drawMode );
    ~DynamicMesh();
    void pushVertices( const vector<Vertex2D>& vertices );
    void pushVertices( const vector<Vertex3D>& vertices );
    void pushVertices( const vector<VertexText3D>& vertices );
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
    void draw();
    template <typename T>
    inline void append( const vector<T>& vertices, vector<GLuint> indices )
    {
      auto base = (GLuint)vertsCount();
      pushVertices( vertices );
      std::for_each( indices.begin(), indices.end(), [base]( GLuint& n ) { n += base; } );
      pushIndices( indices );
    }
  };

  using DynamicMeshPtr = shared_ptr<DynamicMesh>;
  using DynamicMeshVector = vector<DynamicMeshPtr>;

  /*class JSMesh {
  public:
    VBO localVBO_;
    EBO localEBO_;
    BareVAO localVAO_;
    /*void copyFrom( const JSMesh& other )
    {
      localVBO_.type_ = other.localVBO_.type_;
      localVBO_.
    }*
    JSMesh(): localVBO_( VBO_2D ) {}
  };*/

  class JSMesh {
  public:
    VBOPtr vbo_;
    EBOPtr ebo_;
    VAOPtr vao_;
    /*void copyFrom( const JSMesh& other )
    {
      localVBO_.type_ = other.localVBO_.type_;
      localVBO_.
    }*/
    JSMesh() {}
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
    ~StaticMesh();
    void begin();
    void draw();
  };

  using StaticMeshPtr = shared_ptr<StaticMesh>;
  using StaticMeshVector = vector<StaticMeshPtr>;

}