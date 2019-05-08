#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"

namespace neko {

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
      VBO_3D,
      VBO_2D
    } vboType_;
    size_t vbo_;
    GLuint id;
    bool uploaded_;
    size_t size_;
    void draw( GLenum mode );
    VAO( VBOType type, size_t vbo ): vboType_( type ), vbo_( vbo ), id( 0 ), size_( 0 ), uploaded_( false ) {}
  };

  using VAOVector = vector<VAO>;

  class MeshManager {
  private:
    VBOVector<Vertex3D> vbos3d_;
    VBOVector<Vertex2D> vbos2d_;
    VAOVector vaos_;
  public:
    size_t pushVBO( vector<Vertex3D> vertices );
    size_t pushVBO( vector<Vertex2D> vertices );
    void uploadVBOs();
    size_t pushVAO( VAO::VBOType type, size_t verticesVBO );
    VAO& getVAO( size_t index ) { return vaos_[index]; }
    void uploadVAOs();
    void teardown();
  };

}