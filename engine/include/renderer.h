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
#include "viewport.h"
#include "gfx.h"
#include "console.h"

namespace MyGUI {
#ifndef NEKO_NO_GUI
  class NekoPlatform;
#else
  class NekoPlatform
  {
    int dummy;
  };
#endif
}

namespace neko {

  NEKO_EXTERN_CONVAR( dbg_wireframe );

  struct GLInformation
  {
    int32_t versionMajor = 0; //!< Major GL version
    int32_t versionMinor = 0; //!< Minor GL version
    int64_t maxTextureSize = 0; //!< Maximum width/height for GL_TEXTURE
    int64_t maxRenderbufferSize = 0; //!< Maximum width/height for GL_RENDERBUFFER
    int64_t maxFramebufferWidth = 0; //!< Maximum width for GL_FRAMEBUFFER
    int64_t maxFramebufferHeight = 0; //!< Maximum height for GL_FRAMEBUFFER
    int64_t textureBufferAlignment = 0; //!< Minimum alignment for texture buffer sizes and offsets
    int64_t uniformBufferAlignment = 0; //!< Minimum alignment for uniform buffer sizes and offsets
    float maxAnisotropy = 0.0f; //!< Maximum anisotropy level, usually 16.0
    int64_t maxArrayTextureLayers = 0; //!< Maximum array texture layers
    vec3i maxComputeWorkgroupCounts;
    vec3i maxComputeWorkgroupSizes;
    int64_t maxComputeWorkgroupInvocations = 0;
  };

  class MeshNode;
  using MeshNodePtr = shared_ptr<MeshNode>;

  class MeshNode {
  public:
    utf8String name;
    vec3 scale;
    quat rotate;
    vec3 translate;
    vector<Vertex3D> vertices;
    vector<GLuint> indices;
    StaticMeshPtr mesh;
    set<MeshNodePtr> children;
  };

  class Renderer {
    friend class Texture;
    friend class Renderbuffer;
    friend class Framebuffer;
  private:
    struct StaticData {
      MaterialPtr placeholderTexture_;
      StaticMeshPtr screenQuad_;
      StaticMeshPtr screenFourthQuads_[4];
      GLuint emptyVAO_ = 0;
      StaticMeshPtr unitSphere_;
      StaticMeshPtr skybox_;
      StaticMeshPtr unitQuad_;
    } builtin_;
    GLuint implCreateTexture2D( int width, int height,
      GLGraphicsFormat format, GLGraphicsFormat internalFormat,
      GLGraphicsFormat internalType, const void* data,
      GLWrapMode wrap = GLenum::GL_REPEAT,
      Texture::Filtering filtering = Texture::Linear,
      int samples = 1 );
    GLuint implCreateTexture2DArray( int width, int height, int depth,
      GLGraphicsFormat format, GLGraphicsFormat internalFormat,
      GLGraphicsFormat internalType, const void* data,
      GLWrapMode wrap = GLenum::GL_REPEAT,
      Texture::Filtering filtering = Texture::Linear,
      int samples = 1 );
    void implDeleteTexture( GLuint handle );
    GLuint implCreateRenderbuffer( int width, int height, GLGraphicsFormat format, int samples = 1 );
    void implDeleteRenderbuffer( GLuint handle );
    GLuint implCreateFramebuffer( int width, int height );
    void implDeleteFramebuffer( GLuint handle );
    void resetFbo();
  protected:
    GLInformation info_;
    ConsolePtr console_;
    ThreadedLoaderPtr loader_;
    FontManagerPtr fonts_;
    ShadersPtr shaders_;
    MeshManagerPtr meshes_;
#ifndef NEKO_NO_SCRIPTING
    ModelManagerPtr models_;
    TextManagerPtr texts_;
#endif
    platform::RWLock loadLock_;
    MaterialManagerPtr materials_;
    ParticleSystemManagerPtr particles_;
    SpriteManagerPtr sprites_;
    DirectorPtr director_;
    vec2 resolution_;
    struct DrawCtx
    {
      FramebufferPtr fboMainMultisampled_;
      FramebufferPtr fboMain_;
      FramebufferPtr mergedMain_; // the output of fboMain after mergedown & post
      inline bool ready() const noexcept { return ( fboMain_ && fboMain_->available() ); }
    } ctx_;
    struct UserData
    {
      utf8String name_;
      MaterialPtr image_;
    } userData_;
    void implClearAndPrepare( const vec3& color );
    void uploadModelsEnterNode( MeshNodePtr node );
    void sceneDrawEnterNode( MeshNodePtr node, Pipeline& pipeline );
    void prepareSceneDraw( GameTime time, Camera& camera, const ViewportDrawParameters& drawparams );
    void prepareSceneDraw( GameTime time, const ViewportDrawParameters& drawparams );
    void sceneDraw( GameTime time, SManager& scene, Camera& camera, const ViewportDrawParameters& drawparams,
      const RenderVisualizations& vis, bool showVis );
    void clearErrors();
    void setCameraUniforms( Camera& camera, uniforms::Camera& uniform );
    TexturePtr loadPNGTexture( const utf8String& filepath, Texture::Wrapping wrapping, Texture::Filtering filtering );
    void drawSceneRecurse(
      SManager& scene, Camera& cam, const ViewportDrawParameters& drawparams, const RenderVisualizations& vis, bool showVis, c::entity e );
  public:
    Renderer( ThreadedLoaderPtr loader, FontManagerPtr fonts, DirectorPtr director, ConsolePtr console );
    inline const StaticData& builtins() noexcept { return builtin_; }
    void preInitialize();
    void initialize( int width, int height );
    void shutdown();
    TexturePtr createTexture( int width, int height, PixelFormat format, const void* data,
      const Texture::Wrapping wrapping, const Texture::Filtering filtering, int multisamples = 1 );
    MaterialPtr createMaterialWithData( const utf8String& name, int width, int height, PixelFormat format,
      const void* data, const Texture::Wrapping wrapping = Texture::Repeat, const Texture::Filtering filtering = Texture::Linear );
    MaterialPtr createMaterialWithData( const utf8String& name, int width, int height, int depth,
      PixelFormat format, const void* data, const Texture::Wrapping wrapping = Texture::Repeat,
      const Texture::Filtering filtering = Texture::Linear );
    inline MeshManager& meshes() noexcept { return *( meshes_.get() ); }
    inline MaterialManager& materials() noexcept { return *( materials_.get() ); }
    inline ThreadedLoaderPtr loader() noexcept { return loader_; }
    Pipeline& useMaterial( const utf8String& name );
    void bindVao( GLuint id );
    void bindTexture( GLuint unit, TexturePtr texture );
    void bindTextures( const vector<TexturePtr>& textures, GLuint firstUnit = 0 );
    void bindTextures( const vector<GLuint>& textures, GLuint firstUnit = 0 );
    void bindTextureUnits( const vector<GLuint>& textures );
    void bindImageTexture( GLuint unit, TexturePtr texture, int level = 0, GLenum access = gl::GL_READ_WRITE );
    inline TexturePtr getMergedMainFramebuffer()
    {
      if ( ctx_.ready() && ctx_.mergedMain_->available() )
        return ctx_.mergedMain_->texture( 0 );
      return {};
    }
    void update( SManager& scene, GameTime delta, GameTime time );
    void uploadTextures();
    void uploadModels();
    void jsRestart();
    inline Shaders& shaders() noexcept { return *( shaders_.get() ); }
    void drawGame( GameTime time, SManager& scene, Camera& camera, const Viewport* viewport,
      const ViewportDrawParameters& params, const RenderVisualizations& vis, bool showVis );
    void draw( GameTime time, SManager& scene, Camera& camera, const ViewportDrawParameters& drawparams,
      const RenderVisualizations& vis, bool showVis, StaticMeshPtr viewportQuad );
    void reset( int width, int height );
    ~Renderer();
  };

}