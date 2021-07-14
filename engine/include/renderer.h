#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "neko_platform.h"
#include "texture.h"
#include "framebuffer.h"
#include "renderbuffer.h"
#include "materials.h"
#include "meshmanager.h"
#include "modelmanager.h"
#include "scripting.h"
#include "shaders.h"

namespace MyGUI {
  class NekoPlatform;
}

namespace neko {

  struct GLInformation {
    int32_t versionMajor; //!< Major GL version
    int32_t versionMinor; //!< Minor GL version
    int64_t maxTextureSize; //!< Maximum width/height for GL_TEXTURE
    int64_t maxRenderbufferSize; //!< Maximum width/height for GL_RENDERBUFFER
    int64_t maxFramebufferWidth; //!< Maximum width for GL_FRAMEBUFFER
    int64_t maxFramebufferHeight; //!< Maximum height for GL_FRAMEBUFFER
    int64_t textureBufferAlignment; //!< Minimum alignment for texture buffer sizes and offsets
    int64_t uniformBufferAlignment; //!< Minimum alignment for uniform buffer sizes and offsets
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
    struct StaticData {
      MaterialPtr placeholderTexture_;
      StaticMeshPtr screenQuad_;
      GLuint emptyVAO_;
      StaticData(): emptyVAO_( 0 ) {}
    } builtin_;
    GLuint implCreateTexture2D( size_t width, size_t height,
      GLGraphicsFormat format, GLGraphicsFormat internalFormat, GLGraphicsFormat internalType,
      const void* data, GLWrapMode wrap = GLenum::GL_CLAMP_TO_EDGE, Texture::Filtering filtering = Texture::Linear );
    void implDeleteTexture( GLuint handle );
    GLuint implCreateRenderbuffer( size_t width, size_t height, GLGraphicsFormat format );
    void implDeleteRenderbuffer( GLuint handle );
    GLuint implCreateFramebuffer( size_t width, size_t height );
    void implDeleteFramebuffer( GLuint handle );
  protected:
    GLInformation info_;
    ConsolePtr console_;
    ThreadedLoaderPtr loader_;
    FontManagerPtr fonts_;
    shaders::ShadersPtr shaders_;
    MeshManagerPtr meshes_;
    ModelManagerPtr models_;
    platform::RWLock loadLock_;
    MaterialVector materials_;
    DirectorPtr director_;
    void sceneDraw( CameraPtr camera );
    void clearErrors();
  public:
    Renderer( ThreadedLoaderPtr loader, FontManagerPtr fonts, DirectorPtr director, ConsolePtr console );
    void preInitialize();
    void initialize( size_t width, size_t height );
    MaterialPtr createTextureWithData( size_t width, size_t height, PixelFormat format, const void* data, const Texture::Wrapping wrapping = Texture::ClampEdge, const Texture::Filtering filtering = Texture::Linear );
    inline MeshManager& meshes() throw() { return *( meshes_.get() ); }
    void prepare( GameTime time );
    void uploadTextures();
    void jsRestart();
    inline shaders::Shaders& shaders() throw() { return *( shaders_.get() ); }
    void draw( CameraPtr camera, MyGUI::NekoPlatform* gui );
    void reset( size_t width, size_t height );
    ~Renderer();
  };

}