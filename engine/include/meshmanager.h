#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"

namespace neko {

  class EBO;

  enum MeshAttributeIndex: GLuint {
    MeshAttrib_Position = 0,
    MeshAttrib_Texcoord
  };

  //! \class VBO
  //! \brief Vertex Buffer Object
  template <class T>
  class VBO {
  public:
    vector<T> storage_;
    GLuint id;
    bool uploaded;
    VBO(): id( 0 ), uploaded( false ) {}
  };

  template <class T>
  class VBOVector: public vector<VBO<T>> {};

  //! \class VAO
  //! \brief Vertex Array Object
  class VAO {
  public:
    enum VBOType {
      VBO_3D, //!< 0: pos[3], 1: texcoord[2]
      VBO_2D //!< 0: pos[2], 1: texcoord[2]
    } vboType_;
    size_t vbo_;
    GLuint id;
    bool uploaded_;
    size_t size_;
    void draw( GLenum mode );
    VAO( VBOType type, size_t vbo ): vboType_( type ), vbo_( vbo ), id( 0 ), size_( 0 ), uploaded_( false ) {}
  };

  using VAOVector = vector<VAO>;

  class EBO {
  public:
    vector<GLuint> storage_; //!< Contents of this ebo
    GLuint id_; //!< GL "name" for this ebo
    bool uploaded_; //!< Has this been uploaded yet?
    EBO(): id_( 0 ), uploaded_( false ) {}
  };

  using EBOVector = vector<EBO>; //!< Vertex element array

  class MeshManager {
  private:
    VBOVector<Vertex3D> vbos3d_;
    VBOVector<Vertex2D> vbos2d_;
    VAOVector vaos_;
    EBOVector ebos_;
  public:
    size_t pushVBO( vector<Vertex3D> vertices );
    size_t pushVBO( vector<Vertex2D> vertices );
    void uploadVBOs();
    size_t pushVAO( VAO::VBOType type, size_t verticesVBO );
    VAO& getVAO( size_t index ) { return vaos_[index]; }
    void uploadVAOs();
    size_t pushEBO( vector<GLuint> indexes );
    void uploadEBOs();
    void useEBO( size_t index );
    EBO& getEBO( size_t index ) { return ebos_[index]; }
    void teardown();
  };

}