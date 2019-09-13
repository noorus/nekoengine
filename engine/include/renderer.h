#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "neko_platform.h"
#include "textures.h"
#include "materials.h"

namespace neko {

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
    void clearErrors();
  public:
    Renderer( EnginePtr engine );
    void initialize();
    MaterialPtr createTextureWithData( size_t width, size_t height, PixelFormat format, const void* data );
    void prepare( GameTime time );
    void uploadTextures();
    void draw( CameraPtr camera );
    ~Renderer();
  };

}