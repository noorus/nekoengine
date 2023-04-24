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
#include "rainet.h"
#include "viewport.h"

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

  struct ModelLoadOutput
  {
    vector<Vertex3D> vertices_;
    vector<GLuint> indices_;
    StaticMeshPtr mesh_;
  };

  using ModelLoadOutputPtr = shared_ptr<ModelLoadOutput>;

  class SceneNode: public nocopy
  {
  public:
    vec3 translate_;
    vec3 scale_;
    quaternion rotate_;
    utf8String name_;
    ModelLoadOutputPtr mesh_;
    vector<SceneNode*> children_;
    SceneNode* parent_;
    mutable bool needParentUpdate_ : 1;
    bool needChildUpdate_ : 1;
    bool inheritOrientation_ : 1;
    bool inheritScale_ : 1;
    mutable bool cachedOutOfDate_ : 1;
    mutable vec3 derivedTranslate_;
    mutable vec3 derivedScale_;
    mutable quaternion derivedRotate_;
    mutable mat4 cachedTransform_;
    const mat4& getFullTransform() const;
    void updateFromParent() const;
    const quaternion& getDerivedRotate() const;
    const vec3& getDerivedTranslate() const;
    const vec3& getDerivedScale() const;
    void update( bool children, bool parentChanged );
    void setTranslate( const vec3& position );
    void setScale( const vec3& scale );
    void setRotate( const quaternion& rotation );
    void translate( const vec3& position );
    void scale( const vec3& scale );
    void rotate( const quaternion& rotation );
    void needUpdate();
    inline void name( const utf8String& name ) { name_ = name; }
    inline const utf8String& name() const noexcept { return name_; }
    inline const vec3& translation() const noexcept { return translate_; }
    inline const vec3& scaling() const noexcept { return scale_; }
    inline const quaternion& rotation() const noexcept { return rotate_; }
    vec3 convertLocalToWorldPosition( const vec3& localPosition );
    vec3 convertWorldToLocalPosition( const vec3& worldPosition );
    quaternion convertLocalToWorldOrientation( const quaternion& localOrientation );
    quaternion convertWorldToLocalOrientation( const quaternion& worldOrientation );
    SceneNode():
      translate_( 0.0f ),
      rotate_( quatIdentity ),
      scale_( 0.0f ),
      parent_( nullptr ),
      inheritOrientation_( true ),
      inheritScale_( true ),
      needParentUpdate_( true ),
      needChildUpdate_( true ),
      cachedOutOfDate_( true ),
      derivedTranslate_( 0.0f ),
      derivedScale_( 1.0f ),
      derivedRotate_( quatIdentity )
    {
    }
    SceneNode( SceneNode* parent ):
      translate_( 0.0f ),
      rotate_( quatIdentity ),
      scale_( 0.0f ),
      parent_( parent ),
      inheritOrientation_( true ),
      inheritScale_( true ),
      needParentUpdate_( true ),
      needChildUpdate_( true ),
      cachedOutOfDate_( true ),
      derivedTranslate_( 0.0f ),
      derivedScale_( 1.0f ),
      derivedRotate_( quatIdentity )
    {
    }
  };

  class SceneManager: public nocopy {
  protected:
    set<SceneNode*> sceneGraph_;
  public:
    SceneNode* createSceneNode( SceneNode* parent = nullptr );
    void addSceneNode( SceneNode* node );
    void destroySceneNode( SceneNode* node );
    inline set<SceneNode*> sceneGraph() { return sceneGraph_; }
  };

  struct ViewportDrawParameters
  {
    bool drawSky;
    bool drawFBO;
    bool drawWireframe;
    bool isEditor;
    vec2 fullWindowResolution;
  };

  class Renderer: public SceneManager {
    friend class Texture;
    friend class Renderbuffer;
    friend class Framebuffer;
  private:
    struct StaticData {
      MaterialPtr placeholderTexture_;
      StaticMeshPtr screenQuad_;
      StaticMeshPtr screenFourthQuads_[4];
      GLuint emptyVAO_;
      StaticMeshPtr unitSphere_;
      StaticMeshPtr skybox_;
      StaticMeshPtr unitQuad_;
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
    shaders::Pipeline& useMaterial( const utf8String& name );
    void bindVao( GLuint id );
    void bindTexture( GLuint unit, TexturePtr texture );
    void bindTextures( const vector<TexturePtr>& textures, GLuint firstUnit = 0 );
    void bindTextures( const vector<GLuint>& textures, GLuint firstUnit = 0 );
    void bindTextureUnits( const vector<GLuint>& textures );
    void resetFbo();
  protected:
    GLInformation info_;
    ConsolePtr console_;
    ThreadedLoaderPtr loader_;
    FontManagerPtr fonts_;
    shaders::ShadersPtr shaders_;
    MeshManagerPtr meshes_;
#ifndef NEKO_NO_SCRIPTING
    ModelManagerPtr models_;
    TextManagerPtr texts_;
#endif
    platform::RWLock loadLock_;
    MaterialManagerPtr materials_;
    ParticleSystemManagerPtr particles_;
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
    void uploadModelsEnterNode( SceneNode* node );
    void sceneDrawEnterNode( SceneNode* node, shaders::Pipeline& pipeline );
    void sceneDraw( GameTime time, Camera& camera, const ViewportDrawParameters& drawparams );
    void clearErrors();
    void setCameraUniforms( Camera& camera, uniforms::Camera& uniform );
    TexturePtr loadPNGTexture( const utf8String& filepath, Texture::Wrapping wrapping, Texture::Filtering filtering );
  public:
    Renderer( ThreadedLoaderPtr loader, FontManagerPtr fonts, DirectorPtr director, ConsolePtr console );
    inline const StaticData& builtins() noexcept { return builtin_; }
    void preInitialize();
    void initialize( size_t width, size_t height );
    void shutdown();
    MaterialPtr createTextureWithData( const utf8String& name, size_t width, size_t height, PixelFormat format, const void* data, const Texture::Wrapping wrapping = Texture::ClampEdge, const Texture::Filtering filtering = Texture::Linear );
    inline MeshManager& meshes() noexcept { return *( meshes_.get() ); }
    inline TexturePtr getMergedMainFramebuffer()
    {
      if ( ctx_.ready() && ctx_.mergedMain_->available() )
        return ctx_.mergedMain_->texture( 0 );
      return {};
    }
    void update( GameTime delta, GameTime time );
    void uploadTextures();
    void uploadModels();
    void setUserData( uint64_t id, const utf8String name, rainet::Image& image );
    void jsRestart();
    inline shaders::Shaders& shaders() noexcept { return *( shaders_.get() ); }
    void drawGame( GameTime time, Camera& camera, const Viewport* viewport, const ViewportDrawParameters& params );
    void draw(
      GameTime time, Camera& camera, const ViewportDrawParameters& drawparams, StaticMeshPtr viewportQuad );
    void reset( size_t width, size_t height );
    ~Renderer();
  };

}