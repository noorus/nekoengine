#include "stdafx.h"
#include "gfx_types.h"
#include "renderbuffer.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  Renderbuffer::Renderbuffer( Renderer* renderer, size_t width, size_t height, PixelFormat format ):
    Surface( renderer, width, height, format )
  {
    handle_ = renderer_->implCreateRenderbuffer( width_, height_, glFormat_ );
    assert( handle_ );
  }

  Renderbuffer::~Renderbuffer()
  {
    if ( handle_ )
      renderer_->implDeleteRenderbuffer( handle_ );
  }

}