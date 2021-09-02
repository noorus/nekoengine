#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "surface.h"

namespace neko {

  class Renderer;

  //! \class Renderbuffer
  //! \brief A renderbuffer is an image surface optimized for render target usage, meant
  //!   primarily to be used as an attachment to a framebuffer.
  //!   Unlike textures, renderbuffers cannot be sampled and have some other additional limitations.
  //!   Existing data also cannot be uploaded to a renderbuffer, it can only be drawn to on a framebuffer.
  class Renderbuffer: public Surface {
  protected:
    GLsizei multisamples_;
  public:
    inline size_t samples() const throw() { return multisamples_; }
  public:
    Renderbuffer() = delete;
    Renderbuffer( Renderer* renderer, size_t width, size_t height, PixelFormat format, int samples = 0 );
    ~Renderbuffer();
  };

  using RenderbufferPtr = shared_ptr<Renderbuffer>;

}