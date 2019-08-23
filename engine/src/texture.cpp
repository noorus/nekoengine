#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  // for now, hardcoded 2D
  Texture::Texture( Renderer* renderer, size_t width, size_t height, PixelFormat format, const void* data ):
    Surface( renderer, width, height, format ), type_( Tex2D  )
  {
    handle_ = renderer_->implCreateTexture( width_, height_, glFormat_, internalFormat_, internalType_, data );
    assert( handle_ );
  }

  Texture::~Texture()
  {
    if ( handle_ )
      renderer_->implDeleteTexture( handle_ );
  }

}