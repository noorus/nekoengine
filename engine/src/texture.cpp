#include "pch.h"
#include "gfx_types.h"
#include "texture.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  inline GLenum glWrappingFromType( const Texture::Wrapping wrapping )
  {
    return ( wrapping == Texture::ClampEdge        ? GL_CLAMP_TO_EDGE
             : wrapping == Texture::ClampBorder    ? GL_CLAMP_TO_BORDER
             : wrapping == Texture::MirroredRepeat ? GL_MIRRORED_REPEAT
             : wrapping == Texture::Repeat         ? GL_REPEAT
                                                   : GL_MIRROR_CLAMP_TO_EDGE );
  }

  Texture::Texture( Renderer* renderer, int width, int height,
  PixelFormat format, const void* data, const Wrapping wrapping,
  const Filtering filtering, int multisamples ):
  Surface( renderer, width, height, format ),
  type_( multisamples > 1 ? Tex2DMultisample : Tex2D ),
  multisamples_( multisamples )
  {
    handle_ = renderer_->implCreateTexture2D(
      width_, height_, glFormat_, internalFormat_, internalType_,
      data, glWrappingFromType( wrapping ), filtering, multisamples );
    assert( handle_ );
  }

  Texture::Texture( Renderer* renderer, int width, int height, int depth,
    PixelFormat format, const void* data, const Wrapping wrapping,
    const Filtering filtering, int multisamples ):
    Surface( renderer, width, height, depth, format ),
    type_( multisamples > 1 ? Tex2DMultisampleArray : Tex2DArray ), multisamples_( multisamples )
  {
    handle_ = renderer_->implCreateTexture2DArray(
      width_, height_, depth_, glFormat_, internalFormat_, internalType_,
      data, glWrappingFromType( wrapping ), filtering, multisamples );
    assert( handle_ );
  }

  inline void setTextureParameters( GLuint handle, GLWrapMode wrap, Texture::Filtering filtering, const GLInformation& info )
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
      glTextureParameteri( handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );
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

    if ( info.maxAnisotropy > 1.0f )
      glTextureParameterf( handle, GL_TEXTURE_MAX_ANISOTROPY, math::min( info.maxAnisotropy, 16.0f ) );
  }

  //! Called by Texture::Texture()
  GLuint Renderer::implCreateTexture2D( int width, int height,
  GLGraphicsFormat format, GLGraphicsFormat internalFormat, GLGraphicsFormat internalType,
  const void* data, GLWrapMode wrap, Texture::Filtering filtering, int samples )
  {
    assert( width <= info_.maxTextureSize && height <= info_.maxTextureSize );

    GLuint handle = 0;
    glCreateTextures( samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 1, &handle );
    assert( handle != 0 );

    if ( samples < 2 )
      setTextureParameters( handle, wrap, filtering, info_ );

    // 1 byte alignment - i.e. unaligned.
    // Could boost performance to use aligned memory in the future.
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    if ( samples > 1 )
      glTextureStorage2DMultisample( handle, samples, internalFormat, width, height, true );
    else
      glTextureStorage2D( handle, 1, internalFormat, width, height );

    if ( data )
      glTextureSubImage2D( handle, 0, 0, 0, width, height, format, internalType, data );

    if ( filtering == Texture::Mipmapped )
      glGenerateTextureMipmap( handle );

    return handle;
  }

  //! Called by Texture::Texture()
  GLuint Renderer::implCreateTexture2DArray( int width, int height, int depth,
  GLGraphicsFormat format, GLGraphicsFormat internalFormat, GLGraphicsFormat internalType,
  const void* data, GLWrapMode wrap, Texture::Filtering filtering, int samples )
  {
    assert( width <= (size_t)info_.maxTextureSize && height <= (size_t)info_.maxTextureSize );
    assert( depth > 0 && depth < info_.maxArrayTextureLayers );

    GLuint handle = 0;
    glCreateTextures( samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY, 1, &handle );
    assert( handle != 0 );

    if ( samples < 2 )
      setTextureParameters( handle, wrap, filtering, info_ );

    // 1 byte alignment - i.e. unaligned.
    // Could boost performance to use aligned memory in the future.
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    if ( samples > 1 )
      glTextureStorage3DMultisample( handle, samples, internalFormat, width, height, depth, true );
    else
      glTextureStorage3D( handle, 1, internalFormat, width, height, depth );

    if ( data )
      glTextureSubImage3D( handle, 0, 0, 0, 0, width, height, depth, format, internalType, data );

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