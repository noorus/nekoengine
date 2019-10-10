#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  Surface::Surface( Renderer* renderer, size_t width, size_t height, PixelFormat format ):
    width_( width ), height_( height ), format_( format ), handle_( 0 ),
    renderer_( renderer )
  {
    assert( renderer && width > 0 && height > 0 );
    if ( format_ == PixFmtColorRGB8 )
    {
      glFormat_ = GL_RGB8;
      internalFormat_ = GL_RGB;
      internalType_ = GL_UNSIGNED_BYTE;
    }
    else if ( format_ == PixFmtColorRGBA8 )
    {
      glFormat_ = GL_RGBA8;
      internalFormat_ = GL_RGBA;
      internalType_ = GL_UNSIGNED_BYTE;
    }
    else if ( format_ == PixFmtColorRGBA8_A8Input )
    {
      glFormat_ = GL_RGBA;
      internalFormat_ = GL_ALPHA;
      internalType_ = GL_UNSIGNED_BYTE;
    }
    else if ( format_ == PixFmtDepthStencil24_8 )
    {
      glFormat_ = GL_DEPTH24_STENCIL8;
      internalFormat_ = GL_DEPTH_STENCIL;
      internalType_ = GL_UNSIGNED_INT_24_8;
    }
    else if ( format_ == PixFmtColorR8 )
    {
      glFormat_ = GL_RED;
      internalFormat_ = GL_RED;
      internalType_ = GL_UNSIGNED_BYTE;
    }
    else
    {
      NEKO_EXCEPT( "Unsupported or unknown surface format" );
    }
  }

}