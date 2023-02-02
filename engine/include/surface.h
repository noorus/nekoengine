#pragma once
#include "neko_types.h"
#include "gfx_types.h"

namespace neko {

  class Renderer;

  //! \class Surface
  //! \brief A surface.
  class Surface: public nocopy {
  protected:
    size_t width_; //!< Width in pixels.
    size_t height_; //!< Height in pixels.
    PixelFormat format_; //!< Pixel format.
    GLGraphicsFormat glFormat_;
    GLGraphicsFormat internalFormat_;
    GLGraphicsFormat internalType_;
    GLuint handle_; //!< Internal GL handle.
    Renderer* renderer_; //!< Raw pointer should be ok since the renderer should be the owner anyway.
  protected:
    explicit Surface( Renderer* renderer, size_t width, size_t height, PixelFormat format );
  public:
    inline size_t width() const noexcept { return width_; }
    inline size_t height() const noexcept { return height_; }
    inline PixelFormat format() const noexcept { return format_; }
    //! Get the native handle for usage. Do not store it elsewhere so as to not violate RAII.
    inline GLuint handle() const noexcept { return handle_; }
  };

  using SurfacePtr = shared_ptr<Surface>;

}