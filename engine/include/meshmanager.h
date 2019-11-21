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
  public:
    VBOType type_;
    GLuint id_; //!< GL name for this vbo
    bool uploaded_; //!< Has this been uploaded yet?
    bool dirty_; //!< Whether to just sub-update data
    MeshDataModifyHint hint_; //!< What usage hint to use
    vector<float> storage_; //!< Storage buffer
    VBO( const VBOType type ): type_( type ), id_( 0 ), uploaded_( false ), dirty_( false ), hint_( ModifyHint_Often ) {}
    inline const bool empty() const { return storage_.empty(); }

    template <typename T>
    inline void pushVertices( const vector<T>& vertices )
    {
      auto oldSize = storage_.size();
      size_t size = ( vertices.size() * T::element_count );
      storage_.resize( oldSize + size );
      memcpy( (float*)storage_.data() + oldSize, vertices.data(), size * sizeof( float ) );
      dirty_ = true;
    }

    inline const size_t vertexCount() const
    {
      const size_t elements = ( type_ == VBO_2D ? Vertex2D::element_count : type_ == VBO_3D ? Vertex3D::element_count : VertexText3D::element_count );
      return ( storage_.size() / elements );
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
    void draw( GLenum mode, GLsizei size );
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
    void draw();
  };

  using StaticMeshPtr = shared_ptr<StaticMesh>;
  using StaticMeshVector = vector<StaticMeshPtr>;

  class MeshManager: public enable_shared_from_this<MeshManager> {
  private:
    ConsolePtr console_;
    vector<VBOPtr> vbos_[Max_VBOType];
    vector<EBOPtr> ebos_;
    vector<VAOPtr> vaos_;
    DynamicMeshVector dynamics_;
    StaticMeshVector statics_;
  public:
    MeshManager( ConsolePtr console ): console_( move( console ) ) {}
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