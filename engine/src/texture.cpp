#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  Texture::Texture( Renderer* renderer, size_t width, size_t height, PixelFormat format, const void* data, const Wrapping wrapping, const Filtering filtering ):
    Surface( renderer, width, height, format ), type_( Tex2D  )
  {
    GLenum wrap = (
      wrapping == ClampEdge ? GL_CLAMP_TO_EDGE
      : wrapping == ClampBorder ? GL_CLAMP_TO_BORDER
      : wrapping == MirroredRepeat ? GL_MIRRORED_REPEAT
      : wrapping == Repeat ? GL_REPEAT
      : GL_MIRROR_CLAMP_TO_EDGE );
    handle_ = renderer_->implCreateTexture2D( width_, height_, glFormat_, internalFormat_, internalType_, data, wrap, filtering );
    assert( handle_ );
  }

  Texture::~Texture()
  {
    if ( handle_ )
      renderer_->implDeleteTexture( handle_ );
  }

}