#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"
#include "meshmanager.h"
#include "shaders.h"
#include "camera.h"
#include "loader.h"
#include "engine.h"
#include "fontmanager.h"
#include "lodepng.h"

namespace neko {

  using namespace gl;

  namespace BuiltinData {

    const vector<PixelRGBA> placeholderImage2x2 =
    {
      { 255, 0,   0,   255 },
      { 0,   255, 0,   255 },
      { 0,   0,   255, 255 },
      { 255, 0,   255, 255 }
    };

    const vector<Vertex2D> quadStrip2D =
    {   // x      y     s     t
      { -0.5f, -0.5f, 0.0f, 1.0f },
      { 0.5f, -0.5f, 1.0f, 1.0f },
      { -0.5f, 0.5f, 0.0f, 0.0f },
      {  0.5f,  0.5f, 1.0f, 0.0f },
    };

    const vector<Vertex2D> testQuadStrip2D =
    {  // x      y     s     t
    { -50.0f,  50.0f, 0.0f, 0.0f }, // 0
    { -50.0f, -50.0f, 0.0f, 1.0f }, // 1
    {  50.0f, -50.0f, 1.0f, 1.0f }, // 2
    { -50.0f,  50.0f, 0.0f, 0.0f }, // 3
    {  50.0f, -50.0f, 1.0f, 1.0f }, // 4
    {  50.0f,  50.0f, 1.0f, 0.0f }, // 5
    };

    const vector<Vertex2D> screenQuad =
    {   // x      y     s     t
      { -1.0f,  1.0f, 0.0f, 1.0f },
      { -1.0f, -1.0f, 0.0f, 0.0f },
      {  1.0f, -1.0f, 1.0f, 0.0f },
      { -1.0f,  1.0f, 0.0f, 1.0f },
      {  1.0f, -1.0f, 1.0f, 0.0f },
      {  1.0f,  1.0f, 1.0f, 1.0f },
    };

    const vector<GLuint> quadIndices =
    {
      0, 1, 2, 3
    };

  }

  const int64_t c_glVersion[2] = { 4, 5 };

  class DynamicText {
  public:
    DynamicMeshPtr mesh_;
    FontPtr font_;
    MaterialPtr material_;
  public:
    DynamicText( EnginePtr engine, MeshManagerPtr meshmgr, FontManagerPtr fontmgr )
    {
      font_ = fontmgr->createFont();
      engine->loader()->addLoadTask( { LoadTask( font_, R"(data\fonts\LuckiestGuy.ttf)", 32.0f ) } );
      mesh_ = meshmgr->createDynamic( GL_TRIANGLES, VBO_2D );
      engine->console()->printf( Console::srcGfx, "DynamicText: VBO %d, EBO %d, VAO %d", mesh_->vbo_, mesh_->ebo_, mesh_->vao_ );
    }
    inline bool fontLoaded()
    {
      return font_ && font_->loaded_;
    }
    void addText( const utf8String& str, vec2 pen )
    {
      uint32_t prev_codepoint = 0;
      for ( size_t i = 0; i < str.length(); ++i )
      {
        auto codepoint = utils::utf8_to_utf32( &str[i] );
        auto glyph = font_->impl_->getGlyph( codepoint );
        if ( !glyph )
        {
          font_->impl_->loadGlyph( codepoint );
          glyph = font_->impl_->getGlyph( codepoint );
        }
        assert( glyph );
        Real kerning = 0.0f;
        if ( i > 0 )
          kerning = glyph->getKerning( prev_codepoint );
        prev_codepoint = codepoint;
        pen.x += kerning;
        auto p0 = vec2(
          ( pen.x + glyph->offset.x ),
          (int)( pen.y + glyph->offset.y ) );
        auto p1 = vec2(
          ( p0.x + glyph->width ),
          (int)( p0.y - glyph->height )
        );
        auto index = (GLuint)mesh_->vertsCount();
        auto color = vec4( 0.9f, 0.11f, 0.42f, 1.0f );
        vector<Vertex2D> vertices = {
        { p0.x, p1.y, glyph->coords[0].s, glyph->coords[1].t },
        { p0.x, p0.y, glyph->coords[0].s, glyph->coords[0].t },
        { p1.x, p0.y, glyph->coords[1].s, glyph->coords[0].t },
        { p0.x, p1.y, glyph->coords[0].s, glyph->coords[1].t },
        { p1.x, p0.y, glyph->coords[1].s, glyph->coords[0].t },
        { p1.x, p1.y, glyph->coords[1].s, glyph->coords[1].t }
        };
        vector<GLuint> indices = {
          index, index + 1, index + 2, index + 3, index + 4, index + 5
        };
        mesh_->pushIndices( move( indices ) );
        mesh_->pushVertices( move( vertices ) );
        pen.x += glyph->advance.x;
      }
    }
    void updateTexture( Renderer* rndr )
    {
      mesh_->dump( "testText" );
      material_ = rndr->createTextureWithData( font_->impl_->atlas_->width_, font_->impl_->atlas_->height_,
        PixFmtColorR8, (const void*)font_->impl_->atlas_->data_.data(), Texture::ClampBorder, Texture::Mipmapped );
      /*platform::FileWriter writer("debug.png");
      vector<uint8_t> buffer;
      lodepng::encode( buffer, font_->impl_->atlas_->data_, font_->impl_->atlas_->width_, font_->impl_->atlas_->height_, LCT_GREY, 8 );
      writer.writeBlob( buffer.data(), buffer.size() );*/
    }
    void draw()
    {
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, material_->texture_->handle() );
      mesh_->draw();
    }
    ~DynamicText()
    {
      //
    }
  };

  using DynamicTextPtr = shared_ptr<DynamicText>;

  static TexturePtr g_texture;
  static FramebufferPtr g_framebuf;

  static DynamicMeshPtr g_testMesh;
  static DynamicTextPtr g_testText;

  void glStartupFetchAndCheck( GLInformation& info )
  {
    auto glbAuxStr = []( GLenum e ) -> utf8String
    {
      return glbinding::aux::Meta::getString( e );
    };
    auto glvGetI32NoThrow = []( GLenum e  ) -> int32_t
    {
      GLint data = 0;
      glGetIntegerv( e, &data );
      return data;
    };
    auto glvGetI64 = [glbAuxStr]( GLenum e, int64_t& target, bool doNotThrow = false ) -> bool
    {
      GLint64 data = 0;
      glGetInteger64v( e, &data );
      auto error = glGetError();
      if ( error != GL_NO_ERROR )
      {
        if ( !doNotThrow )
          NEKO_OPENGL_EXCEPT( "glGetInteger64v failed for " + glbAuxStr( e ), error );
        return false;
      }
      target = (int64_t)data;
      return true;
    };

    // OpenGL version
    info.versionMajor = glvGetI32NoThrow( GL_MAJOR_VERSION );
    info.versionMinor = glvGetI32NoThrow( GL_MINOR_VERSION );

    // Bail out if the version isn't up to task.
    // The following (newer) calls might otherwise fail in a less controlled way.
    // For example, glGetInteger64v already requires OpenGL 3.2
    if ( info.versionMajor < c_glVersion[0] || ( info.versionMajor == c_glVersion[0] && info.versionMinor < c_glVersion[1] ) )
    {
      // Be all pretty and informative about it.
      std::stringstream description;
      description << "Insufficient OpenGL version!\r\nGot: ";
      if ( info.versionMajor == 0 )
        description << "ancient";
      else
        description << info.versionMajor << "." << info.versionMinor;
      description << "; Expected at least " << c_glVersion[0] << "." << c_glVersion[1];
      NEKO_EXCEPT( description.str() );
    }

    // Max buffer sizes
    glvGetI64( GL_MAX_TEXTURE_SIZE, info.maxTextureSize );
    glvGetI64( GL_MAX_RENDERBUFFER_SIZE, info.maxRenderbufferSize );

    if ( !glvGetI64( GL_MAX_FRAMEBUFFER_WIDTH, info.maxFramebufferWidth, true )
      || !glvGetI64( GL_MAX_FRAMEBUFFER_HEIGHT, info.maxFramebufferHeight, true ) )
    {
      // Some drivers do not support GL_MAX_FRAMEBUFFER_WIDTH/GL_MAX_FRAMEBUFFER_WIDTH
      // despite supporting framebuffers in themselves just fine, due to them being added
      // in a later extension than framebuffers. Let's just default to the max texture size.
      info.maxFramebufferWidth = info.maxTextureSize;
      info.maxFramebufferHeight = info.maxTextureSize;
    }
  }

  void Renderer::clearErrors()
  {
    uint32_t errorCount = 0;
    while ( true )
    {
      auto error = glGetError();
      if ( error == GL_NO_ERROR )
        break;
      if ( error == GL_OUT_OF_MEMORY )
        NEKO_EXCEPT( "Driver generated GL_OUT_OF_MEMORY. Out of memory." );
      errorCount++;
    }
    if ( !errorCount )
      return;
    engine_->console()->printf( Console::srcGfx, "Cleared %d GL errors...", errorCount );
  }

  Renderer::Renderer( EnginePtr engine ): engine_( move( engine ) )
  {
    clearErrors();
    glStartupFetchAndCheck( info_ );
  }

  void Renderer::preInitialize()
  {
    clearErrors();

    shaders_ = make_shared<Shaders>( engine_ );
    shaders_->initialize();

    meshes_ = make_shared<MeshManager>();

    builtin_.placeholderTexture_ = createTextureWithData( 2, 2, PixFmtColorRGBA8,
      (const void*)BuiltinData::placeholderImage2x2.data() );
    builtin_.screenQuad_ = meshes_->createStatic( GL_TRIANGLES, BuiltinData::screenQuad );
  }

  void Renderer::initialize( size_t width, size_t height )
  {
    MaterialPtr myMat = make_shared<Material>();
    materials_.push_back( myMat );
    engine_->loader()->addLoadTask( { LoadTask( myMat, R"(data\textures\test.png)" ) } );

    g_framebuf = make_shared<Framebuffer>( this );

    reset( width, height );
  }

  void Renderer::reset( size_t width, size_t height )
  {
    assert( g_framebuf );
    g_framebuf->recreate( width, height );
  }

  void Renderer::uploadTextures()
  {
    MaterialVector mats;
    engine_->loader()->getFinishedMaterials( mats );
    if ( !mats.empty() )
      engine_->console()->printf( Console::srcGfx, "Renderer::uploadTextures got %d new materials", mats.size() );
    for ( auto& mat : mats )
    {
      if ( !mat->loaded_ )
        continue;
      mat->texture_ = make_shared<Texture>( this, mat->image_.width_, mat->image_.height_, mat->image_.format_, mat->image_.data_.data(), Texture::ClampEdge, Texture::Mipmapped );
    }
  }

  void asdasd( DynamicMeshPtr asd, vector<Vertex2D> verts, vec2 pos )
  {
    for ( auto& v : verts )
    {
      v.x += pos.x;
      v.y += pos.y;
    }
    asd->pushVertices( verts );
    auto index = (GLuint)asd->indicesCount();
    vector<GLuint> indices = { index, index + 1, index + 2, index + 3, index + 4, index + 5 };
    assert( verts.size() == indices.size() );
    asd->pushIndices( indices );
  }

  static bool g_textAdded = false;

  void Renderer::prepare( GameTime time )
  {
    // Upload any new textures. Could this be parallellized?
    uploadTextures();

    // VAOs can and will refer to VBOs and EBOs, and those must have been uploaded by the point at which we try to create the VAO.
    // Thus uploading the VAOs should always come last.
    meshes_->uploadVBOs();
    meshes_->uploadEBOs();
    meshes_->uploadVAOs();

    if ( !g_testText )
    {
      Locator::console().printf( Console::srcGfx, "pt1: Creating g_testText" );
      g_testText = make_shared<DynamicText>( engine_->shared_from_this(), meshes_, engine_->fonts() );
      Locator::console().printf( Console::srcGfx, "pt2: Created g_testText, awaiting font load" );
    }

    if ( g_testText && g_testText->fontLoaded() && !g_textAdded )
    {
      Locator::console().printf( Console::srcGfx, "pt3: Adding text to g_testText" );
      g_textAdded = true;
      g_testText->addText( "nekoengine", vec2( 256.0f, 720.0f - 256.0f + 32.0f ) );
      Locator::console().printf( Console::srcGfx, "pt4: Added text to g_testText" );
      Locator::console().printf( Console::srcGfx, "pt5: Updating texture for g_testText" );
      g_testText->updateTexture( this );
    }

    if ( !g_testMesh && !materials_.empty() && materials_[0]->loaded_ )
    {
      const Real offset = 128.0f;
      g_testMesh = meshes_->createDynamic( GL_TRIANGLES, VBO_2D );
      asdasd( g_testMesh, BuiltinData::testQuadStrip2D, vec2( offset, offset ) );
      asdasd( g_testMesh, BuiltinData::testQuadStrip2D, vec2( 1280.0f - offset, offset ) );
      asdasd( g_testMesh, BuiltinData::testQuadStrip2D, vec2( offset, 720.0f - offset ) );
      asdasd( g_testMesh, BuiltinData::testQuadStrip2D, vec2( 1280.0f - offset, 720.0f - offset ) );
    }
  }

  //! Called by Texture::Texture()
  GLuint Renderer::implCreateTexture2D( size_t width, size_t height,
    GLGraphicsFormat format, GLGraphicsFormat internalFormat, GLGraphicsFormat internalType,
    const void* data, GLWrapMode wrap, Texture::Filtering filtering )
  {
    assert( width <= (size_t)info_.maxTextureSize && height <= (size_t)info_.maxTextureSize );

    GLuint handle = 0;
    glGenTextures( 1, &handle );
    assert( handle != 0 );

    glBindTexture( GL_TEXTURE_2D, handle );

    // Wrapping (repeat, edge, border)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap );

    // Filtering
    if ( filtering == Texture::Nearest )
    {
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }
    else if ( filtering == Texture::Linear )
    {
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    }
    else
    {
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    }

    // 1 byte alignment - i.e. unaligned.
    // Could boost performance to use aligned memory in the future.
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glTexImage2D( GL_TEXTURE_2D, 0, format, (GLsizei)width, (GLsizei)height, 0, internalFormat, internalType, data );

    if ( filtering == Texture::Mipmapped )
      glGenerateMipmap( GL_TEXTURE_2D );

    glBindTexture( GL_TEXTURE_2D, 0 );

    return handle;
  }

  //! Called by Texture::~Texture()
  void Renderer::implDeleteTexture( GLuint handle )
  {
    assert( handle );
    glDeleteTextures( 1, &handle );
  }

  //! Called by Renderbuffer::Renderbuffer()
  GLuint Renderer::implCreateRenderbuffer( size_t width, size_t height, GLGraphicsFormat format )
  {
    assert( width <= (size_t)info_.maxRenderbufferSize && height <= (size_t)info_.maxRenderbufferSize );

    GLuint handle = 0;
    glGenRenderbuffers( 1, &handle );
    assert( handle != 0 );

    glBindRenderbuffer( GL_RENDERBUFFER, handle );
    glRenderbufferStorage( GL_RENDERBUFFER, format, (GLsizei)width, (GLsizei)height );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );

    return handle;
  }

  //! Called by Renderbuffer::~Renderbuffer()
  void Renderer::implDeleteRenderbuffer( GLuint handle )
  {
    assert( handle );
    glDeleteRenderbuffers( 1, &handle );
  }

  //! Called by Framebuffer::create()
  GLuint Renderer::implCreateFramebuffer( size_t width, size_t height )
  {
    assert( width <= (size_t)info_.maxFramebufferWidth && height <= (size_t)info_.maxFramebufferHeight );

    GLuint handle = 0;
    glGenFramebuffers( 1, &handle );
    assert( handle != 0 );

    return handle;
  }

  //! Called by Framebuffer::destroy()
  void Renderer::implDeleteFramebuffer( GLuint handle )
  {
    assert( handle );
    glDeleteFramebuffers( 1, &handle );
  }

  MaterialPtr Renderer::createTextureWithData( size_t width, size_t height, PixelFormat format, const void* data, const Texture::Wrapping wrapping, const Texture::Filtering filtering )
  {
    MaterialPtr mat = make_shared<Material>();
    mat->image_.format_ = format;
    mat->image_.width_ = (unsigned int)width;
    mat->image_.height_ = (unsigned int)height;
    mat->texture_ = make_shared<Texture>( this, mat->image_.width_, mat->image_.height_, mat->image_.format_, data, wrapping, filtering );
    mat->loaded_ = true;
    return move( mat );
  }

  void Renderer::sceneDraw( CameraPtr camera )
  {
    glDisable( GL_DEPTH_TEST );
    glDepthMask( 0 );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    mat4 model( 1.0f );
    model = glm::scale( model, vec3( 1.0f, 1.0f, 1.0f ) );
    model = glm::translate( model, vec3( 1.0f, 1.0f, 0.0f ) );
    shaders_->setMatrices( model, camera->view(), camera->projection() );

    if ( g_testMesh )
    {
      shaders_->use( 0 );

      glActiveTexture( GL_TEXTURE0 );
      if ( !materials_.empty() && materials_[0]->texture_ )
        glBindTexture( GL_TEXTURE_2D, materials_[0]->texture_->handle() );
      else
        glBindTexture( GL_TEXTURE_2D, g_texture->handle() );

      g_testMesh->draw();
    }

    if ( g_testText && g_textAdded )
    {
      shaders_->use( 2 );
      g_testText->draw();
    }
  }

  void Renderer::draw( CameraPtr camera )
  {
    if ( !g_framebuf->available() )
      return;

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );

    glEnable( GL_MULTISAMPLE );

    // Smoothing can generate sub-fragments and cause visible ridges between triangles.
    // Use a framebuffer for AA instead.
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_POLYGON_SMOOTH );

    // Draw the scene inside the framebuffer.
    g_framebuf->begin();
    sceneDraw( camera );
    g_framebuf->end();

    // Framebuffer has been unbound, now draw to the default context, the window.
    glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    shaders_->use( 1 );

    glDisable( GL_DEPTH_TEST );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, g_framebuf->texture()->handle() );
    builtin_.screenQuad_->draw();
  }

  Renderer::~Renderer()
  {
    g_testText.reset();

    g_texture.reset();
    g_framebuf.reset();

    meshes_->teardown();

    shaders_->shutdown();
    shaders_.reset();
  }

}