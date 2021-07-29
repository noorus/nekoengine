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
      glFormat_ = GL_RGB;
      internalFormat_ = GL_RGB8;
      internalType_ = GL_UNSIGNED_BYTE;
    }
    else if ( format_ == PixFmtColorRGBA8 )
    {
      glFormat_ = GL_RGBA;
      internalFormat_ = GL_RGBA8;
      internalType_ = GL_UNSIGNED_BYTE;
    }
    else if ( format_ == PixFmtColorRGBA16f )
    {
      glFormat_ = GL_RGBA;
      internalFormat_ = GL_RGBA16F;
      internalType_ = GL_FLOAT;
    }
    else if ( format_ == PixFmtColorRGBA32f )
    {
      glFormat_ = GL_RGBA;
      internalFormat_ = GL_RGBA32F;
      internalType_ = GL_FLOAT;
    }
    else if ( format_ == PixFmtDepth32f )
    {
      glFormat_ = GL_DEPTH_COMPONENT;
      internalFormat_ = GL_DEPTH_COMPONENT32F;
      internalType_ = GL_FLOAT;
    }
    else if ( format_ == PixFmtColorR8 )
    {
      glFormat_ = GL_RED;
      internalFormat_ = GL_R8;
      internalType_ = GL_UNSIGNED_BYTE;
    }
    else
    {
      NEKO_EXCEPT( "Unsupported or unknown surface format" );
    }
  }

}