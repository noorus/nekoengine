#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "texture.h"
#include "renderbuffer.h"

namespace neko {

  class Renderer;

  //! \class Framebuffer
  //! \brief A framebuffer is an off-screen render target.
  //!   The OpenGL context has one default, final framebuffer, but others can be created.
  //!   Buffers to store the actual data must be bound to the framebuffer separately.
  //!   A framebuffer can have either textures or renderbuffers as its color, depth or stencil attachments.
  //!   A framebuffer is not a surface, since it does not in itself store the buffered data.
  class Framebuffer {
  protected:
    int width_ = 0; //!< Width in pixels.
    int height_ = 0; //!< Height in pixels.
    GLuint handle_ = 0; //!< Internal GL handle.
    Renderer* renderer_; //!< Raw pointer should be ok since the renderer should be the owner anyway.
    vector<TexturePtr> colorBuffers_;
    TexturePtr depthBuffer_;
    size_t colorbufcount_;
    int multisamples_;
    vec4 clearColor_;
    GLuint sampler_ = 0;
    int savedViewport_[4];
    mutable bool available_ = false;
    bool depth_;
    PixelFormat format_;
    bool storedMultisampleEnable_ = false;
  public:
    Framebuffer() = delete;
    Framebuffer( Renderer* renderer, size_t colorBufferCount, PixelFormat colorBufferFormat, bool depthBuffer, int multisamples );
    void recreate( int width, int height );
    void destroy();
    bool validate() const;
    void invalidate();
    void begin();
    void end();
    void prepare( size_t colorReadIndex, vector<size_t> colorWriteIndexes );
    void blitColorTo( size_t sourceIndex, size_t destIndex, Framebuffer& target );
    inline vector<TexturePtr>& textures() { return colorBuffers_; }
    inline TexturePtr texture( size_t index ) { return colorBuffers_.at( index ); }
    inline TexturePtr depth() { return depthBuffer_; }
    bool available() const;
    ~Framebuffer();
  };

  using FramebufferPtr = shared_ptr<Framebuffer>;

}