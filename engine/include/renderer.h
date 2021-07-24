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
    float maxAnisotropy; //!< Maximum anisotropy level, usually 16.0
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
      StaticMeshPtr cube_;
      StaticData(): emptyVAO_( 0 ) {}
    } builtin_;
    GLuint implCreateTexture2D( size_t width, size_t height,
      GLGraphicsFormat format, GLGraphicsFormat internalFormat,
      GLGraphicsFormat internalType, const void* data,
      GLWrapMode wrap = GLenum::GL_CLAMP_TO_EDGE,
      Texture::Filtering filtering = Texture::Linear,
      int samples = 1 );
    void implDeleteTexture( GLuint handle );
    GLuint implCreateRenderbuffer( size_t width, size_t height, GLGraphicsFormat format, int samples = 1 );
    void implDeleteRenderbuffer( GLuint handle );
    GLuint implCreateFramebuffer( size_t width, size_t height );
    void implDeleteFramebuffer( GLuint handle );
    shaders::Pipeline& useMaterial( size_t index );
  protected:
    GLInformation info_;
    ConsolePtr console_;
    ThreadedLoaderPtr loader_;
    FontManagerPtr fonts_;
    shaders::ShadersPtr shaders_;
    MeshManagerPtr meshes_;
#ifndef NEKO_NO_SCRIPTING
    ModelManagerPtr models_;
#endif
    platform::RWLock loadLock_;
    MaterialVector materials_;
    DirectorPtr director_;
    void sceneDraw( Camera& camera );
    void clearErrors();
  protected:
    FramebufferPtr mainbuffer_; //!< Multisampled primary scene render buffer
    FramebufferPtr intermediate_; //!< Intermediate non-multisampled buffer for postprocessing
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
    void draw( GameTime time, Camera& camera, MyGUI::NekoPlatform* gui );
    void reset( size_t width, size_t height );
    ~Renderer();
  };

}