#include "pch.h"
#include "gfx_types.h"
#include "framebuffer.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  const PixelFormat c_depthFormat = PixFmtDepth32f;

  Framebuffer::Framebuffer( Renderer* renderer, size_t colorBufferCount, PixelFormat colorBufferFormat, bool depthBuffer, int multisamples ):
  renderer_( renderer ), width_( 0 ), height_( 0 ), handle_( 0 ), colorbufcount_( colorBufferCount ),
  available_( false ), multisamples_( multisamples ), savedViewport_{ 0 }, depth_( depthBuffer ), format_( colorBufferFormat )
  {
    assert( renderer_ );
    colorBuffers_.resize( colorBufferCount );
    clearColor_ = rgba( 0, 255, 0, 1.0 );
  }

  //! Called by Framebuffer::create()
  GLuint Renderer::implCreateFramebuffer( size_t width, size_t height )
  {
    if ( width > (size_t)info_.maxFramebufferWidth || height > (size_t)info_.maxFramebufferHeight )
      console_->printf( Console::srcGfx,
        "Warning: Requested framebuffer width or height (%i, %i) exceeds maximum supported values (%i, %i)",
        width, height, info_.maxFramebufferWidth, info_.maxFramebufferHeight );

    width = math::min( width, (size_t)info_.maxFramebufferWidth );
    height = math::min( height, (size_t)info_.maxFramebufferHeight );

    GLuint handle = 0;
    glCreateFramebuffers( 1, &handle );
    assert( handle != 0 );

    return handle;
  }

  //! Called by Framebuffer::destroy()
  void Renderer::implDeleteFramebuffer( GLuint handle )
  {
    assert( handle );
    glDeleteFramebuffers( 1, &handle );
  }

#define CONVGLENUM(en, add) (GLenum)((GLuint)en + add)

  void Framebuffer::recreate( size_t width, size_t height )
  {
    assert( width > 0 && height > 0 );

    destroy();

    width_ = width;
    height_ = height;
    handle_ = renderer_->implCreateFramebuffer( width_, height_ );
    assert( handle_ );

    for ( int i = 0; i < colorbufcount_; i++ )
    {
      colorBuffers_.push_back( make_shared<Texture>( renderer_, width_, height_, format_, nullptr, Texture::ClampEdge, Texture::Nearest, multisamples_ ) );
    }

    if ( depth_ )
      depthBuffer_ = make_shared<Texture>( renderer_, width_, height_, c_depthFormat, nullptr, Texture::ClampEdge, Texture::Nearest, multisamples_ );

    assert( colorBuffers_.size() < 32 );
    array<GLenum, 32> drawBuffers;
    for ( unsigned int i = 0; i < colorBuffers_.size(); ++i )
    {
      drawBuffers[i] = CONVGLENUM( GL_COLOR_ATTACHMENT0, i );
      glNamedFramebufferTexture( handle_, drawBuffers[i], colorBuffers_[i]->handle(), 0 );
    }

    glNamedFramebufferDrawBuffers( handle_, (GLsizei)colorBuffers_.size(), drawBuffers.data() );

    if ( depth_ )
      glNamedFramebufferTexture( handle_, GL_DEPTH_ATTACHMENT, depthBuffer_->handle(), 0 );
  }

  void Framebuffer::blitColorTo( size_t sourceIndex, size_t destIndex, Framebuffer& target )
  {
    glNamedFramebufferReadBuffer( handle_, CONVGLENUM( GL_COLOR_ATTACHMENT0, sourceIndex ) );
    // We can do this because we know the indexing is 1:1 due to how we set it up in recreate()
    // The technically correct thing would be to either not touch this on the destination side,
    // or query what the actual value over there would be, but it's the same class so assumptions are fine.
    GLenum tgtbuf = CONVGLENUM( GL_COLOR_ATTACHMENT0, destIndex );
    glNamedFramebufferDrawBuffers( target.handle_, 1, &tgtbuf );
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
    colorBuffers_.clear();
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

  void Framebuffer::prepare( size_t colorReadIndex, vector<size_t> colorWriteIndexes )
  {
    assert( colorReadIndex < colorBuffers_.size() );
    glNamedFramebufferReadBuffer( handle_, CONVGLENUM( GL_COLOR_ATTACHMENT0, colorReadIndex ) );
    array<GLenum, 32> drawBuffers;
    int ctr = 0;
    for ( auto index : colorWriteIndexes )
    {
      assert( index < colorBuffers_.size() );
      drawBuffers[ctr++] = CONVGLENUM( GL_COLOR_ATTACHMENT0, index );
    }
    glNamedFramebufferDrawBuffers( handle_, (GLsizei)colorWriteIndexes.size(), drawBuffers.data() );
  }

  void Framebuffer::beginSimple()
  {
    glNamedFramebufferReadBuffer( handle_, CONVGLENUM( GL_COLOR_ATTACHMENT0, 0 ) );
    glNamedFramebufferDrawBuffer( handle_, CONVGLENUM( GL_COLOR_ATTACHMENT0, 0 ) );
    begin();
  }

  void Framebuffer::begin()
  {
    const GLfloat clearDepth = 0.0f;

    glGetIntegerv( GL_VIEWPORT, savedViewport_ );

    glBindFramebuffer( GL_FRAMEBUFFER, handle_ );
    for ( int i = 0; i < colorBuffers_.size(); i++ )
      glClearNamedFramebufferfv( handle_, GL_COLOR, i, glm::value_ptr( clearColor_ ) );
    if ( depth_ )
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