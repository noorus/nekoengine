#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  Renderbuffer::Renderbuffer( Renderer* renderer, size_t width, size_t height, GLGraphicsFormat format ):
    Surface( renderer, width, height, format )
  {
    handle_ = renderer_->implCreateRenderbuffer( width_, height_, format_ );
    assert( handle_ );
  }

  Renderbuffer::~Renderbuffer()
  {
    if ( handle_ )
      renderer_->implDeleteRenderbuffer( handle_ );
  }

}