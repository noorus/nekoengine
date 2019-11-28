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
    size_t width_; //!< Width in pixels.
    size_t height_; //!< Height in pixels.
    GLuint handle_; //!< Internal GL handle.
    Renderer* renderer_; //!< Raw pointer should be ok since the renderer should be the owner anyway.
    TexturePtr colorBuffer_;
    RenderbufferPtr depthBuffer_;
    vec4 clearColor_;
    mutable bool available_;
  public:
    Framebuffer() = delete;
    Framebuffer( Renderer* renderer );
    void recreate( size_t width, size_t height );
    void destroy();
    bool validate() const;
    void begin();
    void end();
    inline TexturePtr texture() { return colorBuffer_; }
    bool available() const;
    ~Framebuffer();
  };

  using FramebufferPtr = shared_ptr<Framebuffer>;

}