#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  Framebuffer::Framebuffer( Renderer* renderer ):
    renderer_( renderer ), width_( 0 ), height_( 0 ), handle_( 0 ), available_( false )
  {
    assert( renderer_ );
    clearColor_ = vec4( 30.0f / 255.0f, 30.0f / 255.0f, 35.0f / 255.0f, 1.0f );
  }

  void Framebuffer::recreate( size_t width, size_t height )
  {
    assert( width > 0 && height > 0 );

    destroy();

    width_ = width;
    height_ = height;
    handle_ = renderer_->implCreateFramebuffer( width_, height_ );
    assert( handle_ );

    // currently using a static framebuffer format:
    // 8-bit RGB color, 24-bit depth, 8-bit stencil
    colorBuffer_ = make_shared<Texture>( renderer_, width_, height_, PixFmtColorRGB8, nullptr );
    depthStencilBuffer_ = make_shared<Renderbuffer>( renderer_, width_, height_, PixFmtDepthStencil24_8 );

    glBindFramebuffer( GL_FRAMEBUFFER, handle_ );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer_->handle(), 0 );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilBuffer_->handle() );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  }

  void Framebuffer::destroy()
  {
    available_ = false;
    if ( handle_ )
    {
      renderer_->implDeleteFramebuffer( handle_ );
      handle_ = 0;
    }
    colorBuffer_.reset();
    depthStencilBuffer_.reset();
  }

  bool Framebuffer::validate() const
  {
    glBindFramebuffer( GL_FRAMEBUFFER, handle_ );
    bool ret = ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    return ret;
  }

  bool Framebuffer::available() const
  {
    if ( !available_ && handle_ )
    {
      available_ = validate();
    }
    return available_;
  }

  void Framebuffer::begin()
  {
    glBindFramebuffer( GL_FRAMEBUFFER, handle_ );
    glClearColor( clearColor_.r, clearColor_.g, clearColor_.b, clearColor_.a );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
  }

  void Framebuffer::end()
  {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  }

  Framebuffer::~Framebuffer()
  {
    destroy();
  }

}