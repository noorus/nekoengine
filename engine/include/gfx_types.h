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
  struct Vertex2D {
    static const size_t element_count = 4;
    float x, y; //!< Vertex coordinates
    float s, t; //!< Texture coordinates
    Vertex2D(): x( 0.0f ), y( 0.0f ), s( 0.0f ), t( 0.0f ) {}
    Vertex2D( float x_, float y_, float s_, float t_ ): x( x_ ), y( y_ ), s( s_ ), t( t_ ) {}
  };
  struct Vertex3D {
    static const size_t element_count = 19;
    vec3 position; //!< 0: Vertex coordinates
    vec3 normal; //!< 1: Vertex normal
    vec2 texcoord; //!< 2: UV coordinates
    vec4 color; //!< 3: Vertex color
    vec4 tangent; //!< 4: Tangent
    vec3 bitangent; //!< 5: Bitangent
    Vertex3D(): position{ 0.0f }, normal{ 0.0f }, texcoord{ 0.0f }, tangent{ 0.0f }, bitangent{ 0.0f }, color{ 0.0f } {}
    Vertex3D( float x_, float y_, float z_, float nx_, float ny_, float nz_, float s_, float t_, float r_, float g_, float b_, float a_ ):
      position( x_, y_, z_ ), normal( nx_, ny_, nz_ ), texcoord( s_, t_ ), color( r_, g_, b_, a_ ), tangent{ 0.0f }, bitangent{ 0.0f } {}
    Vertex3D( vec3 position_, vec3 normal_, vec2 texcoord_, vec4 color_ ):
      position( move( position_ ) ), normal( move( normal_ ) ), texcoord( move( texcoord_ ) ), color( move( color_ ) ), tangent{ 0.0f }, bitangent{ 0.0f } {}
  };
  struct PixelRGBA {
    uint8_t r, g, b, a;
  };
#pragma pack( pop )

  enum PixelFormat {
    PixFmtColorRGB8,
    PixFmtColorRGBA8,
    PixFmtColorRGBA16f,
    PixFmtColorRGBA32f,
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