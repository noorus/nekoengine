#pragma once
#include <glbinding/gl45core/gl.h>
#include <glbinding/glbinding.h>

namespace neko {

  using gl::GLuint;
  using gl::GLsizei;
  using gl::GLfloat;
  using gl::GLboolean;
  using gl::GLenum;
  using gl::GLchar;
  using gl::GLsizei;
  using gl::GLdouble;
  using gl::GLint;

  //! Some forwards for hinting at usage despite same underlying datatype.
  using GLGraphicsFormat = gl::GLenum;
  using GLWrapMode = gl::GLenum;

  //! Packed structs we use with OpenGL
#pragma pack( push, 1 )
  struct Vertex3D {
    static const size_t element_count = 8;
    vec3 position; //!< Vertex coordinates
    vec3 normal; //!< Vertex normal
    vec2 texcoord; //!< Texture coordinates
    Vertex3D(): position( 0.0f, 0.0f, 0.0f ), normal( 0.0f, 0.0f, 0.0f ), texcoord( 0.0f, 0.0f ) {}
    Vertex3D( float x_, float y_, float z_, float nx_, float ny_, float nz_, float s_, float t_ ):
      position( x_, y_, z_ ), normal( nx_, ny_, nz_ ), texcoord( s_, t_ ) {}
    Vertex3D( vec3 pos, vec3 normal_, vec2 texc ):
      position( move( pos ) ), normal( move( normal_ ) ), texcoord( move( texc ) ) {}
  };
  struct Vertex2D {
    static const size_t element_count = 4;
    float x, y; //!< Vertex coordinates
    float s, t; //!< Texture coordinates
    Vertex2D(): x( 0.0f ), y( 0.0f ), s( 0.0f ), t( 0.0f ) {}
    Vertex2D( float x_, float y_, float s_, float t_ ): x( x_ ), y( y_ ), s( s_ ), t( t_ ) {}
  };
  struct VertexText3D {
    static const size_t element_count = 9;
    vec3 position;
    vec2 texcoord;
    vec4 color;
    VertexText3D(): position( 0.0f, 0.0f, 0.0f ), texcoord( 0.0f, 0.0f ), color( 0.0f, 0.0f, 0.0f, 0.0f ) {}
    VertexText3D( float x_, float y_, float z_, float s_, float t_, float r_, float g_, float b_, float a_ ):
      position( x_, y_, z_ ), texcoord( s_, t_ ), color( r_, g_, b_, a_ ) {}
    VertexText3D( vec3 pos, vec2 texc, vec4 clr ): position( move( pos ) ), texcoord( move( texc ) ), color( move( clr ) ) {}
  };
  struct PixelRGBA {
    uint8_t r, g, b, a;
  };
#pragma pack( pop )

  enum PixelFormat {
    PixFmtColorRGB8,
    PixFmtColorRGBA8,
    PixFmtDepth32f,
    PixFmtColorR8
  };

  struct ImageData {
    unsigned int width_ = 0;
    unsigned int height_ = 0;
    vector<uint8_t> data_;
    PixelFormat format_ = PixFmtColorRGBA8;
  };

}