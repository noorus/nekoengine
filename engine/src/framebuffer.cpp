#include "stdafx.h"
#include "gfx_types.h"
#include "framebuffer.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  Framebuffer::Framebuffer( Renderer* renderer, int multisamples ):
    renderer_( renderer ), width_( 0 ), height_( 0 ), handle_( 0 ),
    available_( false ), multisamples_( multisamples ), savedViewport_{ 0 }
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
    colorBuffer_ = make_shared<Texture>( renderer_, width_, height_, PixFmtColorRGB8, nullptr, Texture::ClampEdge, Texture::Nearest, multisamples_ );
    depthBuffer_ = make_shared<Renderbuffer>( renderer_, width_, height_, PixFmtDepth32f, multisamples_ );

    array<GLenum, 32> drawBuffers;
    auto colorHandle = colorBuffer_->handle();
    for ( unsigned int i = 0; i < 1; ++i )
    {
      drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glNamedFramebufferDrawBuffers( handle_, 1, drawBuffers.data() );

    glNamedFramebufferTexture( handle_, GL_COLOR_ATTACHMENT0, colorBuffer_->handle(), 0 );
    glNamedFramebufferRenderbuffer( handle_, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer_->handle() );
  }

  void Framebuffer::blitColorTo( Framebuffer& target )
  {
    glBlitNamedFramebuffer( handle_, target.handle_,
      0, 0, (GLint)width_, (GLint)height_,
      0, 0, (GLint)target.width_, (GLint)target.height_,
      GL_COLOR_BUFFER_BIT, GL_NEAREST );
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

    glGetIntegerv( GL_VIEWPORT, savedViewport_ );

    glBindFramebuffer( GL_FRAMEBUFFER, handle_ );
    glClearNamedFramebufferfv( handle_, GL_COLOR, 0, glm::value_ptr( clearColor_ ) );
    glClearNamedFramebufferfv( handle_, GL_DEPTH, 0, &clearDepth );

    glViewport( 0, 0, (GLsizei)width_, (GLsizei)height_ );
  }

  void Framebuffer::end()
  {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    glViewport( savedViewport_[0], savedViewport_[1], savedViewport_[2], savedViewport_[3] );
  }

  Framebuffer::~Framebuffer()
  {
    destroy();
  }

}