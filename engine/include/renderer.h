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

  class affine3 {
  protected:
    Real m[4][4];
  public:
    affine3() {}
    affine3( const vec3& position, const quaternion& orientation, const vec3& scale )
    {
      makeTransform( position, scale, orientation );
    }
    template<typename U>
    explicit affine3( const U* ptr )
    {
      for ( int i = 0; i < 3; i++ )
        for ( int j = 0; j < 4; j++ )
          m[i][j] = Real( ptr[i * 4 + j] );
      m[3][0] = 0, m[3][1] = 0, m[3][2] = 0, m[3][3] = 1;
    }
    explicit affine3( const Real* arr )
    {
      m[3][0] = 0, m[3][1] = 0, m[3][2] = 0, m[3][3] = 1;
    }
    affine3(
      Real m00, Real m01, Real m02, Real m03,
      Real m10, Real m11, Real m12, Real m13,
      Real m20, Real m21, Real m22, Real m23 )
    {
      m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
      m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
      m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
      m[3][0] = 0;   m[3][1] = 0;   m[3][2] = 0;   m[3][3] = 1;
    }
    explicit affine3( const mat4& mat )
    {
      m[0][0] = mat[0][0]; m[0][1] = mat[0][1]; m[0][2] = mat[0][2]; m[0][3] = mat[0][3];
      m[1][0] = mat[1][0]; m[1][1] = mat[1][1]; m[1][2] = mat[1][2]; m[1][3] = mat[1][3];
      m[2][0] = mat[2][0]; m[2][1] = mat[2][1]; m[2][2] = mat[2][2]; m[2][3] = mat[2][3];
      m[3][0] = 0;         m[3][1] = 0;         m[3][2] = 0;         m[3][3] = 1;
    }
    affine3& operator = ( const mat3& mat )
    {
      set3x3Matrix( mat );
      return *this;
    }
    const mat4 mat() const
    {
      return mat4(
        m[0][0], m[0][1], m[0][2], m[0][3],
        m[1][0], m[1][1], m[1][2], m[1][3],
        m[2][0], m[2][1], m[2][2], m[2][3],
        m[3][0], m[3][1], m[3][2], m[3][3]
      );
    }
    bool operator == ( const affine3& m2 ) const
    {
      if (
        m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
        m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
        m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] )
        return false;
      return true;
    }
    bool operator != ( const affine3& m2 ) const { return !( *this == m2 ); }
    Real* operator [] ( size_t row )
    {
      return &( m[row][0] );
    }
    const Real* operator [] ( size_t row ) const
    {
      return &( m[row][0] );
    }
    void makeTrans( const vec3& v )
    {
      makeTrans( v.x, v.y, v.z );
    }
    void makeTrans( Real tx, Real ty, Real tz )
    {
      m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = tx;
      m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; m[1][3] = ty;
      m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; m[2][3] = tz;
      m[3][0] = 0.0; m[3][1] = 0.0; m[3][2] = 0.0; m[3][3] = 1.0;
    }
    vec3 getTrans() const
    {
      return vec3( m[0][3], m[1][3], m[2][3] );
    }
    void setTrans( const vec3& v )
    {
      m[0][3] = v[0];
      m[1][3] = v[1];
      m[2][3] = v[2];
    }
    void set3x3Matrix( const mat3& mat )
    {
      m[0][0] = mat[0][0]; m[0][1] = mat[0][1]; m[0][2] = mat[0][2];
      m[1][0] = mat[1][0]; m[1][1] = mat[1][1]; m[1][2] = mat[1][2];
      m[2][0] = mat[2][0]; m[2][1] = mat[2][1]; m[2][2] = mat[2][2];
    }
    vec3 getScale()
    {
      return vec3( m[0][3], m[1][3], m[2][3] );
    }
    void setScale( const vec3& v )
    {
      m[0][0] = v[0];
      m[1][1] = v[1];
      m[2][2] = v[2];
    }
    mat3 linear() const
    {
      return mat3( m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2] );
    }
    affine3 inverse() const
    {
      Real m10 = m[1][0], m11 = m[1][1], m12 = m[1][2];
      Real m20 = m[2][0], m21 = m[2][1], m22 = m[2][2];

      Real t00 = m22 * m11 - m21 * m12;
      Real t10 = m20 * m12 - m22 * m10;
      Real t20 = m21 * m10 - m20 * m11;

      Real m00 = m[0][0], m01 = m[0][1], m02 = m[0][2];

      Real invDet = 1 / ( m00 * t00 + m01 * t10 + m02 * t20 );

      t00 *= invDet;
      t10 *= invDet;
      t20 *= invDet;

      m00 *= invDet;
      m01 *= invDet;
      m02 *= invDet;

      Real r00 = t00;
      Real r01 = m02 * m21 - m01 * m22;
      Real r02 = m01 * m12 - m02 * m11;

      Real r10 = t10;
      Real r11 = m00 * m22 - m02 * m20;
      Real r12 = m02 * m10 - m00 * m12;

      Real r20 = t20;
      Real r21 = m01 * m20 - m00 * m21;
      Real r22 = m00 * m11 - m01 * m10;

      Real m03 = m[0][3], m13 = m[1][3], m23 = m[2][3];

      Real r03 = -( r00 * m03 + r01 * m13 + r02 * m23 );
      Real r13 = -( r10 * m03 + r11 * m13 + r12 * m23 );
      Real r23 = -( r20 * m03 + r21 * m13 + r22 * m23 );

      return affine3(
        r00, r01, r02, r03,
        r10, r11, r12, r13,
        r20, r21, r22, r23 );
    }
    void decomposition( vec3& position, vec3& scale, quaternion& orientation ) const
    {
      vec3 skew;
      vec4 perspective;
      glm::decompose( mat4( linear() ), scale, orientation, position, skew, perspective );
      position = vec3( m[0][3], m[1][3], m[2][3] );
    }
    static Real MINOR( const affine3& m, const size_t r0, const size_t r1, const size_t r2, const size_t c0, const size_t c1, const size_t c2 )
    {
      return m[r0][c0] * ( m[r1][c1] * m[r2][c2] - m[r2][c1] * m[r1][c2] ) -
             m[r0][c1] * ( m[r1][c0] * m[r2][c2] - m[r2][c0] * m[r1][c2] ) +
             m[r0][c2] * ( m[r1][c0] * m[r2][c1] - m[r2][c0] * m[r1][c1] );
    }
    Real determinant() const
    {
      return m[0][0] * MINOR( *this, 1, 2, 3, 1, 2, 3 ) -
             m[0][1] * MINOR( *this, 1, 2, 3, 0, 2, 3 ) +
             m[0][2] * MINOR( *this, 1, 2, 3, 0, 1, 3 ) -
             m[0][3] * MINOR( *this, 1, 2, 3, 0, 1, 2 );
    }
    mat4 transpose() const
    {
      return mat4( m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3] );
    }
    void makeTransform( const vec3& position, const vec3& scale, const quaternion& orientation )
    {
      auto rot3x3 = glm::toMat3( orientation );

      m[0][0] = scale.x * rot3x3[0][0];
      m[0][1] = scale.y * rot3x3[0][1];
      m[0][2] = scale.z * rot3x3[0][2];
      m[0][3] = position.x;
      m[1][0] = scale.x * rot3x3[1][0];
      m[1][1] = scale.y * rot3x3[1][1];
      m[1][2] = scale.z * rot3x3[1][2];
      m[1][3] = position.y;
      m[2][0] = scale.x * rot3x3[2][0];
      m[2][1] = scale.y * rot3x3[2][1];
      m[2][2] = scale.z * rot3x3[2][2];
      m[2][3] = position.z;

      m[3][0] = 0;
      m[3][1] = 0;
      m[3][2] = 0;
      m[3][3] = 1;
    }
    void makeInverseTransform( const vec3& position, const vec3& scale, const quaternion& orientation )
    {
      auto invTranslate = -position;
      vec3 invScale( 1 / scale.x, 1 / scale.y, 1 / scale.z );
      auto invRot = math::inverse( orientation );

      invTranslate = invRot * invTranslate;
      invTranslate *= invScale;

      auto rot3x3 = glm::toMat3( invRot );

      m[0][0] = invScale.x * rot3x3[0][0];
      m[0][1] = invScale.x * rot3x3[0][1];
      m[0][2] = invScale.x * rot3x3[0][2];
      m[0][3] = invTranslate.x;
      m[1][0] = invScale.y * rot3x3[1][0];
      m[1][1] = invScale.y * rot3x3[1][1];
      m[1][2] = invScale.y * rot3x3[1][2];
      m[1][3] = invTranslate.y;
      m[2][0] = invScale.z * rot3x3[2][0];
      m[2][1] = invScale.z * rot3x3[2][1];
      m[2][2] = invScale.z * rot3x3[2][2];
      m[2][3] = invTranslate.z;

      m[3][0] = 0;
      m[3][1] = 0;
      m[3][2] = 0;
      m[3][3] = 1;
    }
  };

  inline affine3 operator * ( const affine3& m, const affine3& m2 )
  {
    return affine3(
      m[0][0] * m2[0][0] + m[0][1] * m2[1][0] + m[0][2] * m2[2][0],
      m[0][0] * m2[0][1] + m[0][1] * m2[1][1] + m[0][2] * m2[2][1],
      m[0][0] * m2[0][2] + m[0][1] * m2[1][2] + m[0][2] * m2[2][2],
      m[0][0] * m2[0][3] + m[0][1] * m2[1][3] + m[0][2] * m2[2][3] + m[0][3],

      m[1][0] * m2[0][0] + m[1][1] * m2[1][0] + m[1][2] * m2[2][0],
      m[1][0] * m2[0][1] + m[1][1] * m2[1][1] + m[1][2] * m2[2][1],
      m[1][0] * m2[0][2] + m[1][1] * m2[1][2] + m[1][2] * m2[2][2],
      m[1][0] * m2[0][3] + m[1][1] * m2[1][3] + m[1][2] * m2[2][3] + m[1][3],

      m[2][0] * m2[0][0] + m[2][1] * m2[1][0] + m[2][2] * m2[2][0],
      m[2][0] * m2[0][1] + m[2][1] * m2[1][1] + m[2][2] * m2[2][1],
      m[2][0] * m2[0][2] + m[2][1] * m2[1][2] + m[2][2] * m2[2][2],
      m[2][0] * m2[0][3] + m[2][1] * m2[1][3] + m[2][2] * m2[2][3] + m[2][3] );
  }

  inline vec3 operator * ( const affine3& m, const vec3& v )
  {
    return vec3(
      m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
      m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
      m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] );
  }

  class SceneNode
  {
  public:
    vec3 translate_;
    vec3 scale_;
    quaternion rotate_;
    utf8String name_;
    shared_ptr<ModelLoadOutput> mesh_;
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
    mutable affine3 cachedTransform_;
    const affine3& getFullTransform() const;
    void updateFromParent() const;
    const quaternion& getDerivedRotate() const;
    const vec3& getDerivedTranslate() const;
    const vec3& getDerivedScale() const;
    void update( bool children, bool parentChanged );
    void setTranslate( const vec3& position );
    void setScale( const vec3& scale );
    void setRotate( const quaternion& rotation );
    void needUpdate();
    vec3 convertLocalToWorldPosition( const vec3& localPosition );
    vec3 convertWorldToLocalPosition( const vec3& worldPosition );
    quaternion convertLocalToWorldOrientation( const quaternion& localOrientation );
    quaternion convertWorldToLocalOrientation( const quaternion& worldOrientation );
    SceneNode()
        : translate_( 0.0f ),
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

  class SceneManager {
  protected:
    set<SceneNode*> sceneGraph_;
  public:
    SceneNode* createSceneNode( SceneNode* parent = nullptr );
    void destroySceneNode( SceneNode* node );
  };

  class Renderer: public SceneManager {
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
    void uploadModelsEnterNode( SceneNode* node );
    void sceneDrawEnterNode( SceneNode* node, shaders::Pipeline& pipeline );
    void sceneDraw( GameTime time, Camera& camera );
    void clearErrors();
    void setCameraUniform( Camera& camera, uniforms::Camera& uniform );
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
    void uploadModels();
    void jsRestart();
    inline shaders::Shaders& shaders() throw() { return *( shaders_.get() ); }
    void draw( GameTime time, Camera& camera, MyGUI::NekoPlatform* gui );
    void reset( size_t width, size_t height );
    ~Renderer();
  };

}