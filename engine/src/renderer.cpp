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
#include "gfx.h"

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

    const vector<Vertex2D> screenQuad2D =
    {   // x      y     s     t
      { -1.0f,  1.0f, 0.0f, 1.0f },
      { -1.0f, -1.0f, 0.0f, 0.0f },
      {  1.0f, -1.0f, 1.0f, 0.0f },
      {  1.0f,  1.0f, 1.0f, 1.0f }
    };

    const vector<GLuint> quadIndices =
    {
      0, 1, 2, 0, 2, 3
    };

  }

  static FramebufferPtr g_framebuf;

  class DynamicText {
  public:
    DynamicMeshPtr mesh_;
    FontPtr font_;
    MaterialPtr material_;
  public:
    DynamicText( ThreadedLoaderPtr loader, MeshManagerPtr meshmgr, FontManagerPtr fontmgr )
    {
      font_ = fontmgr->createFont();
      loader->addLoadTask( { LoadTask( font_, R"(data\fonts\LuckiestGuy.ttf)", 32.0f ) } );
      mesh_ = meshmgr->createDynamic( GL_TRIANGLES, VBO_Text );
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
        auto color = vec4( 0.0f, 1.0f, 0.0f, 1.0f );
        vector<VertexText3D> vertices = {
          { vec3( p0.x, p1.y, 0.0f ), vec2( glyph->coords[0].x, glyph->coords[1].y ), color },
          { vec3( p0.x, p0.y, 0.0f ), vec2( glyph->coords[0].x, glyph->coords[0].y ), color },
          { vec3( p1.x, p0.y, 0.0f ), vec2( glyph->coords[1].x, glyph->coords[0].y ), color },
          { vec3( p1.x, p1.y, 0.0f ), vec2( glyph->coords[1].x, glyph->coords[1].y ), color }
        };
        vector<GLuint> indices = {
          index + 0, index + 1, index + 2, index + 0, index + 2, index + 3
        };
        mesh_->pushIndices( move( indices ) );
        mesh_->pushVertices( move( vertices ) );
        pen.x += glyph->advance.x;
      }
    }
    void updateTexture( Renderer* rndr )
    {
      material_ = rndr->createTextureWithData( font_->impl_->atlas_->width_, font_->impl_->atlas_->height_,
        PixFmtColorR8, (const void*)font_->impl_->atlas_->data_.data(), Texture::ClampBorder, Texture::Mipmapped );
      /* platform::FileWriter writer("debug.png");
      vector<uint8_t> buffer;
      lodepng::encode( buffer, font_->impl_->atlas_->data_, font_->impl_->atlas_->width_, font_->impl_->atlas_->height_, LCT_GREY, 8 );
      writer.writeBlob( buffer.data(), buffer.size() ); */
    }
    void begin()
    {
      mesh_->begin();
    }
    void draw()
    {
      glBindTextureUnit( 0, material_->texture_->handle() );
      mesh_->draw();
    }
    ~DynamicText()
    {
      //
    }
  };

  using DynamicTextPtr = shared_ptr<DynamicText>;

  // static DynamicTextPtr g_testText;

  void glStartupFetchAndCheck( GLInformation& info )
  {
    auto glbAuxStr = []( GLenum e ) -> utf8String
    {
#ifndef RELEASE
      return glbinding::aux::Meta::getString( e );
#else
      return utf8String( std::to_string( (unsigned int)e ) );
#endif
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

    // Uniform buffer alignments
    glvGetI64( GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, info.textureBufferAlignment );
    glvGetI64( GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, info.uniformBufferAlignment );
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
    console_->printf( Console::srcGfx, "Cleared %d GL errors...", errorCount );
  }

  Renderer::Renderer( ThreadedLoaderPtr loader, FontManagerPtr fonts, DirectorPtr director, ConsolePtr console ):
    loader_( move( loader ) ), fonts_( move( fonts ) ), director_( move( director ) ), console_( move( console ) )
  {
    assert( loader_ && fonts_ && director_ && console_ );

    clearErrors();
    glStartupFetchAndCheck( info_ );
  }

  void Renderer::preInitialize()
  {
    clearErrors();

    shaders_ = make_shared<shaders::Shaders>( console_ );
    shaders_->initialize();

    meshes_ = make_shared<MeshManager>( console_ );
    models_ = make_shared<ModelManager>( console_ );

    glCreateVertexArrays( 1, &builtin_.emptyVAO_ );
    builtin_.placeholderTexture_ = createTextureWithData( 2, 2, PixFmtColorRGBA8,
      (const void*)BuiltinData::placeholderImage2x2.data() );
    builtin_.screenQuad_ = meshes_->createStatic( GL_TRIANGLES, BuiltinData::screenQuad2D, BuiltinData::quadIndices );
  }

  void Renderer::initialize( size_t width, size_t height )
  {
    MaterialPtr myMat = make_shared<Material>();
    materials_.push_back( myMat );
    loader_->addLoadTask( { LoadTask( myMat, R"(data\textures\test.png)" ) } );

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
    loader_->getFinishedMaterials( mats );
    if ( !mats.empty() )
      console_->printf( Console::srcGfx, "Renderer::uploadTextures got %d new materials", mats.size() );
    for ( auto& mat : mats )
    {
      if ( !mat->loaded_ )
        continue;
      mat->texture_ = make_shared<Texture>( this,
        mat->image_.width_, mat->image_.height_, mat->image_.format_, mat->image_.data_.data(),
        Texture::ClampEdge, Texture::Mipmapped );
    }
  }

  static bool g_textAdded = false;

  void Renderer::prepare( GameTime time )
  {
    // Upload any new textures. Could this be parallellized?
    uploadTextures();

    meshes_->jsUpdate( director_->renderSync() );

    // VAOs can and will refer to VBOs and EBOs, and those must have been uploaded by the point at which we try to create the VAO.
    // Thus uploading the VAOs should always come last.
    meshes_->uploadVBOs();
    meshes_->uploadEBOs();
    meshes_->uploadVAOs();

    models_->jsUpdate( director_->renderSync() );

    /*if ( !g_testText )
    {
      g_testText = make_shared<DynamicText>( loader_, meshes_, fonts_ );
    }

    if ( g_testText && g_testText->fontLoaded() && !g_textAdded )
    {
      g_textAdded = true;
      g_testText->addText( "nekoengine", vec2( 256.0f, 720.0f - 256.0f + 32.0f ) );
      g_testText->updateTexture( this );
    }*/
  }

  void Renderer::jsRestart()
  {
    console_->printf( Console::srcGfx, "Renderer::jsRestart()" );
    director_->renderSync().resetFromRenderer();
    models_->jsReset();
    meshes_->jsReset();
  }

  //! Called by Texture::Texture()
  GLuint Renderer::implCreateTexture2D( size_t width, size_t height,
    GLGraphicsFormat format, GLGraphicsFormat internalFormat, GLGraphicsFormat internalType,
    const void* data, GLWrapMode wrap, Texture::Filtering filtering )
  {
    assert( width <= (size_t)info_.maxTextureSize && height <= (size_t)info_.maxTextureSize );

    GLuint handle = 0;
    glCreateTextures( GL_TEXTURE_2D, 1, &handle );
    assert( handle != 0 );

    // Wrapping (repeat, edge, border)
    glTextureParameteri( handle, GL_TEXTURE_WRAP_S, wrap );
    glTextureParameteri( handle, GL_TEXTURE_WRAP_T, wrap );

    // Filtering
    if ( filtering == Texture::Nearest )
    {
      glTextureParameteri( handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTextureParameteri( handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }
    else if ( filtering == Texture::Linear )
    {
      glTextureParameteri( handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTextureParameteri( handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    }
    else
    {
      glTextureParameteri( handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      glTextureParameteri( handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    }

    // 1 byte alignment - i.e. unaligned.
    // Could boost performance to use aligned memory in the future.
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glTextureStorage2D( handle, 1, internalFormat, (GLsizei)width, (GLsizei)height );

    if ( data )
      glTextureSubImage2D( handle, 0, 0, 0, (GLsizei)width, (GLsizei)height, format, internalType, data );

    if ( filtering == Texture::Mipmapped )
      glGenerateTextureMipmap( handle );

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
    glCreateRenderbuffers( 1, &handle );
    assert( handle != 0 );

    glNamedRenderbufferStorage( handle, format, (GLsizei)width, (GLsizei)height );

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
    glCreateFramebuffers( 1, &handle );
    assert( handle != 0 );

    return handle;
  }

  //! Called by Framebuffer::destroy()
  void Renderer::implDeleteFramebuffer( GLuint handle )
  {
    assert( handle );
    glDeleteFramebuffers( 1, &handle );
  }

  MaterialPtr Renderer::createTextureWithData( size_t width, size_t height, PixelFormat format,
    const void* data, const Texture::Wrapping wrapping, const Texture::Filtering filtering )
  {
    MaterialPtr mat = make_shared<Material>();
    mat->image_.format_ = format;
    mat->image_.width_ = (unsigned int)width;
    mat->image_.height_ = (unsigned int)height;
    mat->texture_ = make_shared<Texture>( this,
      mat->image_.width_, mat->image_.height_, mat->image_.format_,
      data, wrapping, filtering );
    mat->loaded_ = true;
    return move( mat );
  }

  void Renderer::sceneDraw( CameraPtr camera )
  {
    glDisable( GL_DEPTH_TEST );
    glDepthMask( 0 );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    shaders_->world()->projection = camera->projection();
    shaders_->world()->view = camera->view();

    auto& pl = shaders_->usePipeline( "default3d" );

    // FIXME ridiculously primitive
    // Also, investigate glMultiDrawArraysIndirect
    if ( models_ )
    {
      for ( auto& modelptr : models_->models() )
      {
        if ( !modelptr.second )
          continue;

        auto& model = modelptr.second->model();
        auto mesh = model.mesh_.get();

        if ( !mesh || !mesh->mesh().vao_ || !mesh->mesh().vao_->uploaded_ )
          continue;

        mesh->mesh().vao_->begin();

        mat4 mdl( 1.0f );
        mdl = glm::translate( mdl, model.translate_->v() );
        mdl = glm::scale( mdl, model.scale_->v() );
        mdl *= glm::toMat4( model.rotate_->q() );

        pl.setUniform( "model", mdl );
        pl.setUniform( "tex", 0 );

        if ( !materials_.empty() && materials_[0]->texture_ )
          glBindTextureUnit( 0, materials_[0]->texture_->handle() );
        else
          glBindTextureUnit( 0, builtin_.placeholderTexture_->texture_->handle() );

        auto indices = mesh->mesh().ebo_->storage_.size();
        mesh->mesh().vao_->draw( GL_TRIANGLES, (GLsizei)indices );
      }
    }

    /*if ( g_testText && g_textAdded )
    {
      g_testText->begin();
      mat4 model( 1.0f );
      model = glm::scale( model, vec3( 1.0f, 1.0f, 1.0f ) );
      model = glm::translate( model, vec3( 1.0f, 1.0f, 0.0f ) );

      shaders_->use( "text3d" ).setUniform( "model", model );
      g_testText->draw();
    }*/
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

    // Default to empty VAO, since not having a bound VAO is illegal as per 4.5 spec
    glBindVertexArray( builtin_.emptyVAO_ );

    // Draw the scene inside the framebuffer.
    g_framebuf->begin();
    sceneDraw( camera );
    g_framebuf->end();

    // Framebuffer has been unbound, now draw to the default context, the window.
    glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    builtin_.screenQuad_->begin();
    shaders_->usePipeline( "mainframebuf2d" ).setUniform( "tex", 0 );
    glBindTextureUnit( 0, g_framebuf->texture()->handle() );
    builtin_.screenQuad_->draw();
  }

  Renderer::~Renderer()
  {
    // g_testText.reset();

    g_framebuf.reset();

    models_->teardown();
    meshes_->teardown();

    shaders_->shutdown();
    shaders_.reset();

    if ( builtin_.emptyVAO_ )
      glDeleteVertexArrays( 1, &builtin_.emptyVAO_ );
  }

}