#include "pch.h"
#include "gfx_types.h"
#include "framebuffer.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  const PixelFormat c_depthFormat = PixFmtDepth32f;

  Framebuffer::Framebuffer( Renderer* renderer, size_t colorBufferCount, PixelFormat colorBufferFormat, bool depthBuffer, int multisamples ):
  renderer_( renderer ), colorbufcount_( colorBufferCount ),
  multisamples_( multisamples ), savedViewport_{ 0 }, depth_( depthBuffer ), format_( colorBufferFormat )
  {
    assert( renderer_ );
    colorBuffers_.resize( colorBufferCount );
    clearColor_ = rgba( 0, 255, 0, 1.0 );
  }

  //! Called by Framebuffer::create()
  GLuint Renderer::implCreateFramebuffer( int width, int height )
  {
    if ( width > info_.maxFramebufferWidth || height > info_.maxFramebufferHeight )
      console_->printf( srcGfx,
        "Warning: Requested framebuffer width or height (%i, %i) exceeds maximum supported values (%i, %i)",
        width, height, info_.maxFramebufferWidth, info_.maxFramebufferHeight );

    width = math::min( width, static_cast<int>( info_.maxFramebufferWidth ) );
    height = math::min( height, static_cast<int>( info_.maxFramebufferHeight ) );

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

  void Framebuffer::recreate( int width, int height )
  {
    assert( width > 0 && height > 0 );

    destroy();

    width_ = width;
    height_ = height;
    handle_ = renderer_->implCreateFramebuffer( width_, height_ );
    assert( handle_ );

    if ( !sampler_ )
      glCreateSamplers( 1, &sampler_ );
    assert( sampler_ );

    glSamplerParameteri( sampler_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glSamplerParameteri( sampler_, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glSamplerParameteri( sampler_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glSamplerParameteri( sampler_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glSamplerParameteri( sampler_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    glSamplerParameterfv( sampler_, GL_TEXTURE_BORDER_COLOR, &glm::vec4( 0.0f )[0] );
    glSamplerParameterf( sampler_, GL_TEXTURE_MIN_LOD, -1000.f );
    glSamplerParameterf( sampler_, GL_TEXTURE_MAX_LOD, 1000.f );
    glSamplerParameterf( sampler_, GL_TEXTURE_LOD_BIAS, 0.0f );
    glSamplerParameteri( sampler_, GL_TEXTURE_COMPARE_MODE, GL_NONE );
    glSamplerParameteri( sampler_, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );

    for ( int i = 0; i < colorbufcount_; i++ )
    {
      colorBuffers_.push_back( make_shared<Texture>(
        renderer_, width_, height_, format_, nullptr, Texture::Repeat, Texture::Nearest, multisamples_ ) );
    }

    if ( depth_ )
      depthBuffer_ = make_shared<Texture>(
        renderer_, width_, height_, c_depthFormat, nullptr, Texture::Repeat, Texture::Nearest, multisamples_ );

    assert( colorBuffers_.size() < 32 );
    array<GLenum, 32> drawBuffers {};
    for ( unsigned int i = 0; i < colorBuffers_.size(); ++i )
    {
      drawBuffers[i] = CONVGLENUM( GL_COLOR_ATTACHMENT0, i );
      glNamedFramebufferTexture( handle_, drawBuffers[i], colorBuffers_[i]->handle(), 0 );
    }

    glNamedFramebufferDrawBuffers( handle_, (GLsizei)colorBuffers_.size(), drawBuffers.data() );

    if ( depth_ )
      glNamedFramebufferTexture( handle_, GL_DEPTH_ATTACHMENT, depthBuffer_->handle(), 0 );

    GLint samples = 0;
    glGetNamedFramebufferParameteriv( handle_, GL_SAMPLES, &samples );
    if ( samples != multisamples_ )
      NEKO_EXCEPT( "Framebuffer's GL_SAMPLES does not match attachment's multisample value" );
  }

  void Framebuffer::blitColorTo( size_t sourceIndex, size_t destIndex, Framebuffer& target )
  {
    //glNamedFramebufferReadBuffer( handle_, CONVGLENUM( GL_COLOR_ATTACHMENT0, sourceIndex ) );
    // We can do this because we know the indexing is 1:1 due to how we set it up in recreate()
    // The technically correct thing would be to either not touch this on the destination side,
    // or query what the actual value over there would be, but it's the same class so assumptions are fine.
    GLenum tgtbuf = CONVGLENUM( GL_COLOR_ATTACHMENT0, destIndex );
    //glNamedFramebufferDrawBuffers( target.handle_, 1, &tgtbuf );
    glBlitNamedFramebuffer( handle_, target.handle_,
      0, 0, (GLint)width_, (GLint)height_,
      0, 0, (GLint)target.width_, (GLint)target.height_,
      GL_COLOR_BUFFER_BIT, GL_NEAREST );
  }

  void Framebuffer::invalidate()
  {
    vector<GLenum> indices {};
    for ( int i = 0; i < colorBuffers_.size(); ++i )
      indices.push_back( CONVGLENUM( GL_COLOR_ATTACHMENT0, i ) );
    if ( depth_ )
      indices.push_back( GL_DEPTH_ATTACHMENT );
    glInvalidateNamedFramebufferData( handle_, static_cast<GLsizei>( indices.size() ), indices.data() );
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
    if ( sampler_ )
    {
      glDeleteSamplers( 1, &sampler_ );
      sampler_ = 0;
    }
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
    // glNamedFramebufferReadBuffer( handle_, CONVGLENUM( GL_COLOR_ATTACHMENT0, colorReadIndex ) );
    array<GLenum, 32> drawBuffers {};
    int ctr = 0;
    for ( auto index : colorWriteIndexes )
    {
      assert( index < colorBuffers_.size() );
      drawBuffers[ctr++] = CONVGLENUM( GL_COLOR_ATTACHMENT0, index );
    }
    // glNamedFramebufferDrawBuffers( handle_, (GLsizei)colorWriteIndexes.size(), drawBuffers.data() );
  }

  void Framebuffer::begin()
  {
    if ( multisamples_ > 1 )
    {
      storedMultisampleEnable_ = glIsEnabled( GL_MULTISAMPLE ) ? true : false;
      glEnable( GL_MULTISAMPLE );
    }

    glClipControl( GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE );

    glGetIntegerv( GL_VIEWPORT, savedViewport_ );
    glViewportIndexedf( 0, 0.0f, 0.0f, static_cast<GLfloat>( width_ ), static_cast<GLfloat>( height_ ) );

    const GLfloat clearDepth = 0.0f;

    for ( int i = 0; i < colorBuffers_.size(); i++ )
      glClearNamedFramebufferfv( handle_, GL_COLOR, i, glm::value_ptr( clearColor_ ) );
    if ( depth_ )
      glClearNamedFramebufferfv( handle_, GL_DEPTH, 0, &clearDepth );

    glBindFramebuffer( GL_FRAMEBUFFER, handle_ );
    glBindSamplers( 0, 1, &sampler_ );
  }

  void Framebuffer::end()
  {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    if ( multisamples_ > 1 && !storedMultisampleEnable_ )
      glDisable( GL_MULTISAMPLE );

    glViewportIndexedf( 0, static_cast<GLfloat>( savedViewport_[0] ), static_cast<GLfloat>( savedViewport_[1] ),
      static_cast<GLfloat>( savedViewport_[2] ), static_cast<GLfloat>( savedViewport_[3] ) );
  }

  Framebuffer::~Framebuffer()
  {
    destroy();
  }

}