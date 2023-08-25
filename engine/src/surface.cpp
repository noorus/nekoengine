#include "pch.h"
#include "gfx_types.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  // clang-format off

  const map<PixelFormat, PixelFormatValues> c_pixelFormatData =
  {
    { PixFmtColorRGB8,
      {
        .components = 3,
        .bytes = 3,
        .glformat = GL_RGB,
        .internalformat = GL_RGB8,
        .gltype = GL_UNSIGNED_BYTE
      }
    },
    { PixFmtColorRGBA8,
      {
        .components = 4,
        .bytes = 4,
        .glformat = GL_RGBA,
        .internalformat = GL_RGBA8,
        .gltype = GL_UNSIGNED_BYTE
      }
    },
    { PixFmtColorRGBA16f,
      {
        .components = 4,
        .bytes = 8,
        .glformat = GL_RGBA,
        .internalformat = GL_RGBA16F,
        .gltype = GL_HALF_FLOAT
      }
    },
    { PixFmtColorRGBA32f,
      {
        .components = 4,
        .bytes = 16,
        .glformat = GL_RGBA,
        .internalformat = GL_RGBA32F,
        .gltype = GL_FLOAT
      }
    },
    { PixFmtDepth32f,
      {
        .components = 1,
        .bytes = 4,
        .glformat = GL_DEPTH_COMPONENT,
        .internalformat = GL_DEPTH_COMPONENT32F,
        .gltype = GL_FLOAT
      }
    },
    { PixFmtDepth24,
      {
        .components = 1,
        .bytes = 3,
        .glformat = GL_DEPTH_COMPONENT,
        .internalformat = GL_DEPTH_COMPONENT24,
        .gltype = GL_UNSIGNED_INT
      }
    },
    { PixFmtDepth16,
      {
        .components = 1,
        .bytes = 2,
        .glformat = GL_DEPTH_COMPONENT,
        .internalformat = GL_DEPTH_COMPONENT16,
        .gltype = GL_UNSIGNED_SHORT
      }
    },
    { PixFmtDepth24Stencil8,
      {
        .components = 1,
        .bytes = 4,
        .glformat = GL_DEPTH_STENCIL,
        .internalformat = GL_DEPTH24_STENCIL8,
        .gltype = GL_UNSIGNED_INT_24_8
      }
    },
    { PixFmtColorR8,
      {
        .components = 1,
        .bytes = 1,
        .glformat = GL_RED,
        .internalformat = GL_R8,
        .gltype = GL_UNSIGNED_BYTE
      }
    },
    { PixFmtColorRG8,
      {
        .components = 2,
        .bytes = 2,
        .glformat = GL_RG,
        .internalformat = GL_RG8,
        .gltype = GL_UNSIGNED_BYTE
      }
    },
    { PixFmtColorR16f,
      {
        .components = 1,
        .bytes = 2,
        .glformat = GL_RED,
        .internalformat = GL_R16F,
        .gltype = GL_HALF_FLOAT
      }
    }
  };

  Surface::Surface( Renderer* renderer, int width, int height, PixelFormat format ):
    width_( width ), height_( height ), format_( format ), handle_( 0 ), renderer_( renderer )
  {
    assert( renderer && width > 0 && height > 0 );
    replaceFormats( format );
  }

  Surface::Surface( Renderer* renderer, int width, int height, int depth, PixelFormat format ):
    width_( width ), height_( height ), depth_( depth ), format_( format ), handle_( 0 ),
    renderer_( renderer )
  {
    assert( renderer && width > 0 && height > 0 && depth > 0 );
    replaceFormats( format );
  }

  void Surface::replaceFormats( PixelFormat newfmt )
  {
    format_ = newfmt;

    if ( !c_pixelFormatData.contains( format_ ) )
      NEKO_EXCEPT( "Unsupported or unknown surface format" );

    const auto& fmt = c_pixelFormatData.at( format_ );
    glFormat_ = fmt.glformat;
    internalFormat_ = fmt.internalformat;
    internalType_ = fmt.gltype;
  }

}