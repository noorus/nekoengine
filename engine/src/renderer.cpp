#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  Renderer::Renderer()
  {
    //
  }

  GLuint Renderer::implCreateTexture( size_t width, size_t height, GLGraphicsFormat format, const void* data )
  {
    GLuint handle;
    glGenTextures( 1, &handle );
    assert( handle != 0 );

    glBindTexture( GL_TEXTURE_2D, handle );

    // assumptions for now
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    // assumptions for now
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // glGenerateMipmap( GL_TEXTURE_2D );

    // 1 byte alignment
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glTexImage2D( GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

    glBindTexture( GL_TEXTURE_2D, 0 );

    return handle;
  }

  void Renderer::implDeleteTexture( GLuint handle )
  {
    assert( handle );
    glDeleteTextures( 1, &handle );
  }

  GLuint Renderer::implCreateRenderbuffer( size_t width, size_t height, GLGraphicsFormat format )
  {
    assert( width <= GL_MAX_RENDERBUFFER_SIZE && height <= GL_MAX_RENDERBUFFER_SIZE );

    GLuint handle;
    glGenRenderbuffers( 1, &handle );
    assert( handle != 0 );

    glBindRenderbuffer( GL_RENDERBUFFER, handle );
    glRenderbufferStorage( GL_RENDERBUFFER, format, (GLsizei)width, (GLsizei)height );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );

    return handle;
  }

  void Renderer::implDeleteRenderbuffer( GLuint handle )
  {
    assert( handle );
    glDeleteRenderbuffers( 1, &handle );
  }

  Renderer::~Renderer()
  {
    //
  }

}