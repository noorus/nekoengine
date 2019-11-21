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
    // 8-bit RGB color, 32-bit float depth
    colorBuffer_ = make_shared<Texture>( renderer_, width_, height_, PixFmtColorRGB8, nullptr, Texture::ClampEdge, Texture::Nearest );
    depthBuffer_ = make_shared<Renderbuffer>( renderer_, width_, height_, PixFmtDepth32f );

    glNamedFramebufferTexture( handle_, GL_COLOR_ATTACHMENT0, colorBuffer_->handle(), 0 );
    glNamedFramebufferRenderbuffer( handle_, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer_->handle() );
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
    depthBuffer_.reset();
  }

  bool Framebuffer::validate() const
  {
    return ( glCheckNamedFramebufferStatus( handle_, GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
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
    const GLfloat clearDepth = 0.0f;
    glBindFramebuffer( GL_FRAMEBUFFER, handle_ );
    glClearNamedFramebufferfv( handle_, GL_COLOR, 0, glm::value_ptr( clearColor_ ) );
    glClearNamedFramebufferfv( handle_, GL_DEPTH, 0, &clearDepth );
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