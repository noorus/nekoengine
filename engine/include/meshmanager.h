#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"

namespace neko {

  class EBO;

  enum VBOType {
    VBO_3D, //!< 0: pos[3], 1: texcoord[2]
    VBO_2D //!< 0: pos[2], 1: texcoord[2]
  };

  enum MeshAttributeIndex: GLuint {
    MeshAttrib_Position = 0,
    MeshAttrib_Texcoord
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
  class VBO {
  public:
    vector<T> storage_;
    GLuint id;
    bool uploaded;
    bool dirty_; //!< Whether to just sub-update data
    MeshDataModifyHint hint_; //!< What usage hint to use
    VBO(): id( 0 ), uploaded( false ), dirty_( false ), hint_( ModifyHint_Often ) {}
  };

  template <class T>
  class VBOVector: public vector<VBO<T>> {};

  //! \class VAO
  //! \brief Vertex Array Object
  //! An object that describes structure of variables in a VBO, referenceable by an ID.
  class VAO {
  public:
    VBOType vboType_; //!< Type of the associated VBO
    size_t vbo_; //!< Index of the associated VBO
    size_t ebo_;
    GLuint id;
    bool uploaded_;
    bool useEBO_;
    size_t size_;
    void draw( GLenum mode );
    VAO( VBOType type, size_t vbo ): vboType_( type ),
      vbo_( vbo ), ebo_( 0 ), id( 0 ), size_( 0 ),
      uploaded_( false ), useEBO_( false ) {}
    VAO( VBOType type, size_t vbo, size_t ebo ): vboType_( type ),
      vbo_( vbo ), ebo_( ebo ), id( 0 ), size_( 0 ),
      uploaded_( false ), useEBO_( true ) {}
  };

  using VAOVector = vector<VAO>;

  //! \class EBO
  //! \brief Element Buffer Object
  //! An array of indices into a VBO.
  class EBO {
  public:
    vector<GLuint> storage_; //!< Contents of this ebo
    GLuint id_; //!< GL "name" for this ebo
    bool uploaded_; //!< Has this been uploaded yet?
    bool dirty_; //!< Whether to just sub-update data
    MeshDataModifyHint hint_; //!< What usage hint to use
    EBO(): id_( 0 ), uploaded_( false ), dirty_( false ), hint_( ModifyHint_Often ) {}
  };

  using EBOVector = vector<EBO>;

  class MeshManager {
  private:
    VBOVector<Vertex2D> vbos2d_;
    VBOVector<Vertex3D> vbos3d_;
    VAOVector vaos_;
    EBOVector ebos_;
  public:
    size_t pushVBO( vector<Vertex3D> vertices );
    size_t pushVBO( vector<Vertex2D> vertices );
    void uploadVBOs();
    inline VBO<Vertex2D>& getVBO2D( size_t index ) { return vbos2d_[index]; }
    inline VBO<Vertex3D>& getVBO3D( size_t index ) { return vbos3d_[index]; }
    size_t pushVAO( VBOType type, size_t verticesVBO );
    size_t pushVAO( VBOType type, size_t verticesVBO, size_t indicesEBO );
    void uploadVAOs();
    inline VAO& getVAO( size_t index ) { return vaos_[index]; }
    size_t pushEBO( vector<GLuint> indexes );
    void uploadEBOs();
    inline EBO& getEBO( size_t index ) { return ebos_[index]; }
    void teardown();
  };

}