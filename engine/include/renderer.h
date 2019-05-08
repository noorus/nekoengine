#pragma once
#include "neko_types.h"
#include "gfx_types.h"

namespace neko {

  class Renderer;

  //! \class Surface
  //! \brief A surface.
  class Surface {
  protected:
    size_t width_;
    size_t height_;
    GLGraphicsFormat format_;
    GLuint handle_;
    Renderer* renderer_; //!< Raw pointer should be ok since the renderer should be the owner anyway.
  protected:
    explicit Surface( Renderer* renderer, size_t width, size_t height, GLGraphicsFormat format );
  public:
    Surface() = delete;
    inline size_t width() const throw() { return width_; }
    inline size_t height() const throw() { return height_; }
    inline GLGraphicsFormat format() const throw() { return format_; }
    //! Get the native handle for usage. Don't store it elsewhere so as to not violate RAII.
    inline GLuint handle() const throw() { return handle_; }
  };

  //! \class Texture
  //! \brief A texture is the fundamental resource encapsulating one (or sometimes more) images
  //!   to be sampled while drawing primitives. Textures (as single GPU units) can vary
  //!   from a 1D pixel strip to an array of cubenamps, and can be created with any graphics format.
  //!   Textures are the most flexible surface/buffer primitive, being able to be uploaded, downloaded,
  //!   sampled, and modified.
  class Texture: public Surface {
  public:
    enum Type {
      Tex1D,
      Tex2D,
      Tex3D,
      Tex1DArray,
      Tex2DArray,
      Tex3DArray,
      TexCubemap,
      TexCubemapArray,
      Tex2DMultisample,
      Tex2DMultisampleArray
    };
  protected:
    Type type_;
  public:
    Texture() = delete;
    Texture( Renderer* renderer, size_t width, size_t height, GLGraphicsFormat format, const void* data );
    ~Texture();
  };

  using TexturePtr = shared_ptr<Texture>;

  //! \class Renderbuffer
  //! \brief A renderbuffer is an image surface optimized for render target usage, meant
  //!   primarily to be used as an attachment to a framebuffer.
  //!   Unlike textures, renderbuffers cannot be sampled and have some other additional limitations.
  //!   Existing data also cannot be uploaded to a renderbuffer, it can only be drawn to on a framebuffer.
  class Renderbuffer: public Surface {
  public:
    Renderbuffer() = delete;
    Renderbuffer( Renderer* renderer, size_t width, size_t height, GLGraphicsFormat format );
    ~Renderbuffer();
  };

  using RenderbufferPtr = shared_ptr<Renderbuffer>;

  //! \class Framebuffer
  //! \brief A framebuffer is an off-screen render target.
  //!   The OpenGL context has one default, final framebuffer, but others can be created.
  //!   Buffers to store the actual data must be bound to the framebuffer separately.
  //!   A framebuffer can have either textures or renderbuffers as its color, depth or stencil attachments.
  //class Framebuffer {
    //
  //};

  //using FramebufferPtr = shared_ptr<Framebuffer>;

  class Renderer {
    friend class Texture;
    friend class Renderbuffer;
    //friend class Framebuffer;
  private:
    GLuint implCreateTexture( size_t width, size_t height, GLGraphicsFormat format, const void* data );
    void implDeleteTexture( GLuint handle );
    GLuint implCreateRenderbuffer( size_t width, size_t height, GLGraphicsFormat format );
    void implDeleteRenderbuffer( GLuint handle );
  protected:
    EnginePtr engine_;
    ShadersPtr shaders_;
    MeshManagerPtr meshes_;
  public:
    Renderer( EnginePtr engine );
    void draw( CameraPtr camera );
    ~Renderer();
  };

}