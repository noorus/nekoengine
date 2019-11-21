#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "utilities.h"
#include "neko_pooledtypes.h"

namespace neko {

  class EBO;

  const size_t cInvalidIndexValue = numeric_limits<size_t>::max();

  enum VBOType {
    VBO_3D, //!< 0: pos[3], 1: texcoord[2]
    VBO_2D, //!< 0: pos[2], 1: texcoord[2]
    VBO_Text //!< 0: pos[3], 1: texcoord[2], 2: color[4]
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
  template <class T>
  class VBO: public PooledVectorObject {
  public:
    vector<T> storage_;
    GLuint id_;
    bool uploaded_;
    bool dirty_; //!< Whether to just sub-update data
    MeshDataModifyHint hint_; //!< What usage hint to use
    VBO(): id_( 0 ), uploaded_( false ), dirty_( false ), hint_( ModifyHint_Often ) {}
    inline const bool empty() const { return storage_.empty(); }
    void release() override
    {
      storage_.clear();
      id_ = 0;
      uploaded_ = false;
      dirty_ = false;
      hint_ = ModifyHint_Often;
    }
  };

  template <class T>
  class VBOVector: public PooledVector<VBO<T>> {};

  //! \class VAO
  //! \brief Vertex Array Object
  //! An object that describes structure of variables in a VBO, referenceable by an ID.
  class VAO: public PooledVectorObject {
  public:
    VBOType vboType_; //!< Type of the associated VBO
    size_t vbo_; //!< Index of the associated VBO
    size_t ebo_;
    GLuint id_;
    bool uploaded_;
    bool useEBO_;
    void draw( GLenum mode, GLsizei size );
    // Construct empty (basically, unused (for PooledVector)
    VAO(): vboType_( VBO_3D ), vbo_( cInvalidIndexValue ), ebo_( cInvalidIndexValue ), id_( 0 ),
      uploaded_( false ), useEBO_( false ) {}
    // Construct with VBO, not using EBO
    VAO( VBOType type, size_t vbo ): vboType_( type ),
      vbo_( vbo ), ebo_( cInvalidIndexValue ), id_( 0 ),
      uploaded_( false ), useEBO_( false ) {}
    // Construct with VBO & EBO
    VAO( VBOType type, size_t vbo, size_t ebo ): vboType_( type ),
      vbo_( vbo ), ebo_( ebo ), id_( 0 ),
      uploaded_( false ), useEBO_( true ) {}
    void release() override
    {
      vboType_ = VBO_3D;
      vbo_ = cInvalidIndexValue;
      ebo_ = cInvalidIndexValue;
      id_ = 0;
      uploaded_ = false;
      useEBO_ = false;
    }
  };

  using VAOVector = PooledVector<VAO>;

  //! \class EBO
  //! \brief Element Buffer Object
  //! An array of indices into a VBO.
  class EBO: public PooledVectorObject {
  public:
    vector<GLuint> storage_; //!< Contents of this ebo
    GLuint id_; //!< GL "name" for this ebo
    bool uploaded_; //!< Has this been uploaded yet?
    bool dirty_; //!< Whether to just sub-update data
    MeshDataModifyHint hint_; //!< What usage hint to use
    EBO(): id_( 0 ), uploaded_( false ), dirty_( false ), hint_( ModifyHint_Often ) {}
    inline const bool empty() const { return storage_.empty(); }
    void release() override
    {
      storage_.clear();
      id_ = 0;
      uploaded_ = false;
      dirty_ = false;
      hint_ = ModifyHint_Often;
    }
  };

  using EBOVector = PooledVector<EBO>;

  class DynamicMesh {
  public:
    VBOType vertexType_;
    MeshManagerPtr manager_;
    size_t vbo_;
    size_t ebo_;
    size_t vao_;
    GLenum drawMode_;
  public:
    DynamicMesh( MeshManagerPtr manager, VBOType vertexType, GLenum drawMode );
    void pushVertices( vector<Vertex2D> verts );
    void pushVertices( vector<Vertex3D> verts );
    void pushVertices( vector<VertexText3D> verts );
    void pushIndices( vector<GLuint> indices );
    const size_t vertsCount() const;
    const size_t indicesCount() const;
    void dump( const utf8String& name );
    void draw();
    ~DynamicMesh();
  };

  using DynamicMeshPtr = shared_ptr<DynamicMesh>;
  using DynamicMeshVector = vector<DynamicMeshPtr>;

  class StaticMesh {
  private:
    MeshManagerPtr manager_;
    size_t vbo_;
    size_t ebo_;
    size_t vao_;
    GLenum drawMode_;
    GLsizei size_;
  public:
    StaticMesh( MeshManagerPtr manager, GLenum drawMode, vector<Vertex2D> verts );
    void draw();
    ~StaticMesh();
  };

  using StaticMeshPtr = shared_ptr<StaticMesh>;
  using StaticMeshVector = vector<StaticMeshPtr>;

  class MeshManager: public enable_shared_from_this<MeshManager> {
  private:
    VBOVector<Vertex2D> vbos2d_;
    VBOVector<Vertex3D> vbos3d_;
    VBOVector<VertexText3D> vbosText3d_;
    VAOVector vaos_;
    EBOVector ebos_;
    DynamicMeshVector dynamics_;
    StaticMeshVector statics_;
  public:
    size_t pushVBO( vector<Vertex3D> vertices );
    size_t pushVBO( vector<Vertex2D> vertices );
    void uploadVBOs();
    inline VBO<Vertex2D>& getVBO2D( size_t index ) { return vbos2d_[index]; }
    inline VBO<Vertex3D>& getVBO3D( size_t index ) { return vbos3d_[index]; }
    inline VBO<VertexText3D>& getVBOText( size_t index ) { return vbosText3d_[index]; }
    size_t pushVAO( VBOType type, size_t verticesVBO );
    size_t pushVAO( VBOType type, size_t verticesVBO, size_t indicesEBO );
    void uploadVAOs();
    inline VAO& getVAO( size_t index ) { return vaos_[index]; }
    size_t pushEBO( vector<GLuint> indices );
    void uploadEBOs();
    inline EBO& getEBO( size_t index ) { return ebos_[index]; }
    size_t createVBO( VBOType type );
    void freeVBO( VBOType type, size_t id );
    size_t createEBO();
    void freeEBO( size_t id );
    size_t createVAO();
    void freeVAO( size_t id );
    DynamicMeshPtr createDynamic( GLenum drawMode, VBOType vertexType );
    StaticMeshPtr createStatic( GLenum drawMode, vector<Vertex2D> verts );
    void teardown();
  };

}