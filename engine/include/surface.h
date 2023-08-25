#pragma once
#include "neko_types.h"
#include "gfx_types.h"

namespace neko {

  class Renderer;

  //! \class Surface
  //! \brief A surface.
  class Surface: public nocopy {
  protected:
    int width_; //!< Width in pixels.
    int height_; //!< Height in pixels.
    int depth_ = 0;
    PixelFormat format_; //!< Pixel format.
    GLGraphicsFormat glFormat_;
    GLGraphicsFormat internalFormat_;
    GLGraphicsFormat internalType_;
    GLuint handle_; //!< Internal GL handle.
    Renderer* renderer_; //!< Raw pointer should be ok since the renderer should be the owner anyway.
    void replaceFormats( PixelFormat newfmt );
  protected:
    explicit Surface( Renderer* renderer, int width, int height, PixelFormat format );
    explicit Surface( Renderer* renderer, int width, int height, int depth, PixelFormat format );
  public:
    inline int width() const noexcept { return width_; }
    inline int height() const noexcept { return height_; }
    inline int depth() const noexcept { return depth_; }
    inline PixelFormat format() const noexcept { return format_; }
    // GLGraphicsFormat glformat() const noexcept { return glFormat_; }
    GLGraphicsFormat internalFormat() const noexcept { return internalFormat_; }
    //! Get the native handle for usage. Do not store it elsewhere so as to not violate RAII.
    inline GLuint handle() const noexcept { return handle_; }
  };

  using SurfacePtr = shared_ptr<Surface>;

}