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
    std::variant<vector<Vertex2D>,vector<Vertex3D>,vector<VertexText3D>> store_;
  public:
    const VBOType type_; //!< Vertex format
    GLuint id_; //!< GL name for this vbo
    bool uploaded_; //!< Has this been uploaded yet?
    bool dirty_; //!< Whether to just sub-update data
    MeshDataModifyHint hint_; //!< What usage hint to use

    explicit VBO( const VBOType type ): type_( type ), id_( 0 ), uploaded_( false ), dirty_( false ), hint_( ModifyHint_Often )
    {
      switch ( type_ )
      {
        case VBO_2D: store_ = vector<Vertex2D>(); break;
        case VBO_3D: store_ = vector<Vertex3D>(); break;
        case VBO_Text: store_ = vector<VertexText3D>(); break;
        default: NEKO_EXCEPT( "Bad VBO type" ); break;
      }
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
    inline vector<VertexText3D>& vt3d()
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

  class MeshGenerator {
  public:
    void makePlane( DynamicMesh& mesh, vec2 dimensions, vec2u segments, vec3 normal );
  };

  class MeshManager: public enable_shared_from_this<MeshManager> {
  private:
    ConsolePtr console_;
    vector<VBOPtr> vbos_[Max_VBOType];
    vector<EBOPtr> ebos_;
    vector<VAOPtr> vaos_;
    DynamicMeshVector dynamics_;
    StaticMeshVector statics_;
    MeshGenerator generator_;
  public:
    MeshManager( ConsolePtr console ): console_( move( console ) ) {}
    inline MeshGenerator& generator() { return generator_; }
    VBOPtr pushVBO( const vector<Vertex3D>& vertices );
    VBOPtr pushVBO( const vector<Vertex2D>& vertices );
    VBOPtr pushVBO( const vector<VertexText3D>& vertices );
    void uploadVBOs();
    VAOPtr pushVAO( VBOPtr verticesVBO );
    VAOPtr pushVAO( VBOPtr verticesVBO, EBOPtr indicesEBO );
    void uploadVAOs();
    EBOPtr pushEBO( const vector<GLuint>& indices );
    void uploadEBOs();
    VBOPtr createVBO( VBOType type );
    void freeVBO( VBOPtr vbo );
    EBOPtr createEBO();
    void freeEBO( EBOPtr ebo );
    VAOPtr createVAO();
    void freeVAO( VAOPtr vao );
    DynamicMeshPtr createDynamic( GLenum drawMode, VBOType vertexType );
    StaticMeshPtr createStatic( GLenum drawMode, vector<Vertex2D> verts );
    StaticMeshPtr createStatic( GLenum drawMode, vector<Vertex2D> verts, vector<GLuint> indices );
    void teardown();
  };

}