#include "pch.h"
#include "gfx_types.h"
#include "texture.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  Texture::Texture( Renderer* renderer, size_t width, size_t height,
  PixelFormat format, const void* data, const Wrapping wrapping,
  const Filtering filtering, int multisamples ):
  Surface( renderer, width, height, format ),
  type_( multisamples > 1 ? Tex2DMultisample : Tex2D ),
  multisamples_( multisamples )
  {
    GLenum wrap = (
      wrapping == ClampEdge ? GL_CLAMP_TO_EDGE
      : wrapping == ClampBorder ? GL_CLAMP_TO_BORDER
      : wrapping == MirroredRepeat ? GL_MIRRORED_REPEAT
      : wrapping == Repeat ? GL_REPEAT
      : GL_MIRROR_CLAMP_TO_EDGE );
    handle_ = renderer_->implCreateTexture2D( width_, height_,
      glFormat_, internalFormat_, internalType_, data,
      wrap, filtering, multisamples );
    assert( handle_ );
  }

  //! Called by Texture::Texture()
  GLuint Renderer::implCreateTexture2D( size_t width, size_t height,
  GLGraphicsFormat format, GLGraphicsFormat internalFormat, GLGraphicsFormat internalType,
  const void* data, GLWrapMode wrap, Texture::Filtering filtering, int samples )
  {
    assert( width <= (size_t)info_.maxTextureSize && height <= (size_t)info_.maxTextureSize );

    GLuint handle = 0;
    glCreateTextures( samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 1, &handle );
    assert( handle != 0 );

    if ( samples < 2 )
    {
      // Wrapping (repeat, edge, border)
      glTextureParameteri( handle, GL_TEXTURE_WRAP_S, wrap );
      glTextureParameteri( handle, GL_TEXTURE_WRAP_T, wrap );

      if ( wrap == GL_CLAMP_TO_BORDER )
      {
        // Set border color (to transparent)
        float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        glTextureParameterfv( handle, GL_TEXTURE_BORDER_COLOR, color );
      }

      // Filtering
      if ( filtering == Texture::Nearest )
      {
        glTextureParameteri( handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTextureParameteri( handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      }
      else if ( filtering == Texture::Linear )
      {
        glTextureParameteri( handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTextureParameteri( handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      }
      else
      {
        glTextureParameteri( handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTextureParameteri( handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      }

      if ( info_.maxAnisotropy > 1.0f )
        glTextureParameterf( handle, GL_TEXTURE_MAX_ANISOTROPY, math::min( info_.maxAnisotropy, 16.0f ) );
    }

    // 1 byte alignment - i.e. unaligned.
    // Could boost performance to use aligned memory in the future.
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    if ( samples > 1 )
      glTextureStorage2DMultisample( handle, samples, internalFormat, (GLsizei)width, (GLsizei)height, true );
    else
      glTextureStorage2D( handle, 1, internalFormat, (GLsizei)width, (GLsizei)height );

    if ( data )
      glTextureSubImage2D( handle, 0, 0, 0, (GLsizei)width, (GLsizei)height, format, internalType, data );

    if ( filtering == Texture::Mipmapped )
      glGenerateTextureMipmap( handle );

    return handle;
  }

  //! Called by Texture::~Texture()
  void Renderer::implDeleteTexture( GLuint handle )
  {
    assert( handle );
    glDeleteTextures( 1, &handle );
  }

  Texture::~Texture()
  {
    if ( handle_ )
      renderer_->implDeleteTexture( handle_ );
  }

}