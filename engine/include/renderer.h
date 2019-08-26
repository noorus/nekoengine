#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "neko_platform.h"

namespace neko {

  class Renderer;

  struct GLInformation {
    int32_t versionMajor; //!< Major GL version
    int32_t versionMinor; //!< Minor GL version
    int64_t maxTextureSize; //!< Maximum width/height for GL_TEXTURE
    int64_t maxRenderbufferSize; //!< Maximum width/height for GL_RENDERBUFFER
    int64_t maxFramebufferWidth; //!< Maximum width for GL_FRAMEBUFFER
    int64_t maxFramebufferHeight; //!< Maximum height for GL_FRAMEBUFFER
    GLInformation()
    {
      memset( this, 0, sizeof( GLInformation ) );
    }
  };

  //! \class Surface
  //! \brief A surface.
  class Surface {
  public:
    enum PixelFormat {
      PixFmtColorRGB8,
      PixFmtColorRGBA8,
      PixFmtDepthStencil24_8
    };
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
    Surface() = delete;
    inline size_t width() const throw() { return width_; }
    inline size_t height() const throw() { return height_; }
    inline PixelFormat format() const throw() { return format_; }
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
    Texture( Renderer* renderer, size_t width, size_t height, PixelFormat format, const void* data );
    ~Texture();
  };

  using TexturePtr = shared_ptr<Texture>;

  struct ImageData {
    unsigned int width_;
    unsigned int height_;
    vector<uint8_t> data_;
    Surface::PixelFormat format_;
  };

  struct Material {
    bool loaded_;
    ImageData image_;
    TexturePtr texture_;
    Material(): loaded_( false ) {}
  };

  using MaterialPtr = shared_ptr<Material>;

  using MaterialVector = vector<MaterialPtr>;

  //! \class Renderbuffer
  //! \brief A renderbuffer is an image surface optimized for render target usage, meant
  //!   primarily to be used as an attachment to a framebuffer.
  //!   Unlike textures, renderbuffers cannot be sampled and have some other additional limitations.
  //!   Existing data also cannot be uploaded to a renderbuffer, it can only be drawn to on a framebuffer.
  class Renderbuffer: public Surface {
  public:
    Renderbuffer() = delete;
    Renderbuffer( Renderer* renderer, size_t width, size_t height, PixelFormat format );
    ~Renderbuffer();
  };

  using RenderbufferPtr = shared_ptr<Renderbuffer>;

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
    RenderbufferPtr depthStencilBuffer_;
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

  class Renderer {
    friend class Texture;
    friend class Renderbuffer;
    friend class Framebuffer;
  private:
    GLuint implCreateTexture( size_t width, size_t height,
      GLGraphicsFormat format, GLGraphicsFormat internalFormat, GLGraphicsFormat internalType,
      const void* data );
    void implDeleteTexture( GLuint handle );
    GLuint implCreateRenderbuffer( size_t width, size_t height, GLGraphicsFormat format );
    void implDeleteRenderbuffer( GLuint handle );
    GLuint implCreateFramebuffer( size_t width, size_t height );
    void implDeleteFramebuffer( GLuint handle );
  protected:
    GLInformation info_;
    EnginePtr engine_;
    ShadersPtr shaders_;
    MeshManagerPtr meshes_;
    platform::RWLock loadLock_;
    MaterialVector materials_;
    void sceneDraw( CameraPtr camera );
  public:
    Renderer( EnginePtr engine );
    void initialize();
    void uploadTextures();
    void draw( CameraPtr camera );
    ~Renderer();
  };

}