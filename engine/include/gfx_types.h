#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "camera.h"

namespace neko {

#pragma pack( push, 1 )
  struct Vertex3D {
    float x, y, z; //!< Vertex coordinates
    float s, t; //!< Texture coordinates
  };
  struct Vertex2D {
    float x, y; //!< Vertex coordinates
    float s, t; //!< Texture coordinates
  };
  struct PixelRGBA {
    uint8_t r, g, b, a;
  };
#pragma pack( pop )

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

  using GLGraphicsFormat = gl::GLenum;

}