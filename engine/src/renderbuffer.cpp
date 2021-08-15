#include "stdafx.h"
#include "gfx_types.h"
#include "renderbuffer.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  Renderbuffer::Renderbuffer( Renderer* renderer, size_t width, size_t height, PixelFormat format, int samples ):
  Surface( renderer, width, height, format ), multisamples_( samples )
  {
    handle_ = renderer_->implCreateRenderbuffer( width_, height_, glFormat_, multisamples_ );
    assert( handle_ );
  }

  //! Called by Renderbuffer::Renderbuffer()
  GLuint Renderer::implCreateRenderbuffer( size_t width, size_t height, GLGraphicsFormat format, int samples )
  {
    assert( width <= (size_t)info_.maxRenderbufferSize && height <= (size_t)info_.maxRenderbufferSize );

    GLuint handle = 0;
    glCreateRenderbuffers( 1, &handle );
    assert( handle != 0 );

    if ( samples > 1 )
      glNamedRenderbufferStorageMultisample( handle, samples, format, (GLsizei)width, (GLsizei)height );
    else
      glNamedRenderbufferStorage( handle, format, (GLsizei)width, (GLsizei)height );

    return handle;
  }

  //! Called by Renderbuffer::~Renderbuffer()
  void Renderer::implDeleteRenderbuffer( GLuint handle )
  {
    assert( handle );
    glDeleteRenderbuffers( 1, &handle );
  }

  Renderbuffer::~Renderbuffer()
  {
    if ( handle_ )
      renderer_->implDeleteRenderbuffer( handle_ );
  }

}