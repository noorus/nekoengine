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

  NEKO_DECLARE_CONVAR( vid_msaa, "Main buffer multisample antialiasing multiplier.", 8 );
  NEKO_DECLARE_CONVAR( dbg_shownormals, "Whether to visualize vertex normals with lines.", true );
  NEKO_DECLARE_CONVAR( dbg_showtangents, "Whether to visualize vertex tangents with lines.", true );
  NEKO_DECLARE_CONVAR( vid_hdr, "Toggle HDR processing.", true );
  NEKO_DECLARE_CONVAR( vid_gamma, "Screen gamma target.", 2.2f );
  NEKO_DECLARE_CONVAR( vid_exposure, "Testing.", 1.0f );

  namespace BuiltinData {

    const vector<PixelRGBA> placeholderImage2x2 =
    {
      { 255, 0,   0,   255 },
      { 0,   255, 0,   255 },
      { 0,   0,   255, 255 },
      { 255, 0,   255, 255 }
    };

    const vector<Vertex2D> screenQuad2D =
    {  // x      y     s     t
      { -1.0f,  1.0f, 0.0f, 1.0f },
      { -1.0f, -1.0f, 0.0f, 0.0f },
      {  1.0f, -1.0f, 1.0f, 0.0f },
      {  1.0f,  1.0f, 1.0f, 1.0f }
    };

    const vector<GLuint> quadIndices =
    {
      0, 1, 2, 0, 2, 3
    };

    static vector<Vertex3D> worldUnitCube3D =
    {  // x      y      z      nx     ny     nz     s     t
      { -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      { -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },

      { -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      { -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },

      { -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      { -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      { -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      { -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },

      {  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },

      { -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      { -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },

      { -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      {  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
      { -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f }
    };

    const vector<GLuint> cubeIndices =
    {
      0,  1,  2,  2,  3,  0,
      4,  5,  6,  6,  7,  4,
      8,  9,  10, 10, 11, 8,
      12, 13, 14, 14, 15, 12,
      16, 17, 18, 18, 19, 16,
      20, 21, 22, 22, 23, 20
    };

  }

#pragma pack( push, 1 )
  struct VertexPointRender {
    vec3 pos;
    vec4 color;
  };
#pragma pack( pop )

  class PointRenderBuffer {
  protected:
    const GLuint c_maxVertices = 32;
    unique_ptr<SmarterBuffer<VertexPointRender>> buffer_;
    GLuint vao_;
  public:
    PointRenderBuffer()
    {
      buffer_ = make_unique<SmarterBuffer<VertexPointRender>>( c_maxVertices );
      glCreateVertexArrays( 1, &vao_ );
      neko::AttribWriter attribs;
      attribs.add( GL_FLOAT, 3 ); // vec3 position
      attribs.add( GL_FLOAT, 4 ); // vec4 color
      attribs.write( vao_ );
      glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs.stride() );
    }
    inline SmarterBuffer<VertexPointRender>& buffer() { return *buffer_.get(); }
    void draw( shaders::Shaders& shaders, GLsizei count, GLint base = 0 )
    {
      glBindVertexArray( vao_ );
      auto& pipeline = shaders.usePipeline( "pointlight" );
      mat4 mdl( 1.0f );
      pipeline.setUniform( "model", mdl );
      // glEnable( GL_PROGRAM_POINT_SIZE );
      glDrawArrays( GL_POINTS, base, count );
      glBindVertexArray( 0 );
    }
    ~PointRenderBuffer()
    {
      glDeleteVertexArrays( 1, &vao_ );
      buffer_.reset();
    }
  };

  static unique_ptr<PointRenderBuffer> g_pointrender;

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
      mesh_ = meshmgr->createDynamic( GL_TRIANGLES, VBO_3D, true, false );
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
        auto normal = vec3( 1.0f, 0.0f, 0.0f );
        auto color = vec4( 0.0f, 1.0f, 0.0f, 1.0f );
        vector<Vertex3D> vertices = {
          { vec3( p0.x, p1.y, 0.0f ), normal, vec2( glyph->coords[0].x, glyph->coords[1].y ), color },
          { vec3( p0.x, p0.y, 0.0f ), normal, vec2( glyph->coords[0].x, glyph->coords[0].y ), color },
          { vec3( p1.x, p0.y, 0.0f ), normal, vec2( glyph->coords[1].x, glyph->coords[0].y ), color },
          { vec3( p1.x, p1.y, 0.0f ), normal, vec2( glyph->coords[1].x, glyph->coords[1].y ), color }
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
      glBindTextureUnit( 0, material_->layers_[0].texture_->handle() );
      mesh_->draw();
      glBindTextureUnit( 0, 0 );
    }
    ~DynamicText()
    {
      //
    }
  };

  using DynamicTextPtr = shared_ptr<DynamicText>;

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
    auto glvGetI32NoThrow = []( GLenum e ) -> int32_t
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
    auto glvGetFloat = [glbAuxStr]( GLenum e, float& target, bool doNotThrow = false ) -> bool
    {
      GLfloat data = 0.0f;
      glGetFloatv( e, &data );
      auto error = glGetError();
      if ( error != GL_NO_ERROR )
      {
        if ( !doNotThrow )
          NEKO_OPENGL_EXCEPT( "glGetFloatv failed for " + glbAuxStr( e ), error );
        return false;
      }
      target = (float)data;
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

    // Maximum anistropic filtering level
    if ( !glvGetFloat( GL_MAX_TEXTURE_MAX_ANISOTROPY, info.maxAnisotropy, true ) )
      info.maxAnisotropy = 1.0f;

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

    console_->printf( Console::srcGfx, "Buffer alignments are %i/texture %i/uniform",
      info_.textureBufferAlignment, info_.uniformBufferAlignment );
    console_->printf( Console::srcGfx, "Maximums are %i/texture %i/renderbuffer %ix%i/framebuffer",
      info_.maxTextureSize, info_.maxRenderbufferSize, info_.maxFramebufferWidth, info_.maxFramebufferHeight );
    console_->printf( Console::srcGfx, "Maximum anisotropy is %.1f", info_.maxAnisotropy );
  }

  void Renderer::preInitialize()
  {
    clearErrors();

    shaders_ = make_shared<shaders::Shaders>( console_ );
    shaders_->initialize();

    meshes_ = make_shared<MeshManager>( console_ );
#ifndef NEKO_NO_SCRIPTING
    models_ = make_shared<ModelManager>( console_ );
#endif

    glCreateVertexArrays( 1, &builtin_.emptyVAO_ );
    builtin_.placeholderTexture_ = createTextureWithData( 2, 2, PixFmtColorRGBA8,
      (const void*)BuiltinData::placeholderImage2x2.data() );
    builtin_.screenQuad_ = meshes_->createStatic( GL_TRIANGLES, BuiltinData::screenQuad2D, BuiltinData::quadIndices );
    util::generateTangentsAndBitangents( BuiltinData::worldUnitCube3D, BuiltinData::cubeIndices );
    builtin_.cube_ = meshes_->createStatic( GL_TRIANGLES, BuiltinData::worldUnitCube3D, BuiltinData::cubeIndices );
  }

  void Renderer::initialize( size_t width, size_t height )
  {
    materials_.push_back( make_shared<Material>( Material::UnlitSimple ) );
    materials_.push_back( make_shared<Material>( Material::WorldPBR ) );
    materials_.push_back( make_shared<Material>( Material::WorldPBR ) );
    loader_->addLoadTask( { LoadTask( materials_[0], { R"(data\textures\test.png)" } ) } );
    loader_->addLoadTask( { LoadTask( materials_[1], {
      R"(data\textures\SGT_Ground_1_AlbedoSmoothness.png)",
      R"(data\textures\SGT_Ground_1_Height.png)",
      R"(data\textures\SGT_Ground_1_MetallicSmoothness.png)",
      R"(data\textures\SGT_Ground_1_Normal.png)" } ) } );
    loader_->addLoadTask( { LoadTask( materials_[2], {
      R"(data\textures\M_Tank_Tiger_Base_AlbedoTransparency.png)",
      R"(data\textures\M_Tank_Tiger_Base_MetallicSmoothness.png)",
      R"(data\textures\M_Tank_Tiger_Metal_AlbedoTransparency.png)",
      R"(data\textures\M_Tank_Tiger_Base_Normal.png)" } ) } );
    loader_->addLoadTask( { LoadTask( new SceneNode(), R"(data\meshes\new_tank_tiger.fbx)" ) } );

    // loader_->addLoadTask( { LoadTask( R"(data\meshes\SCA_Aircraft_Flight.anim)"

    mainbuffer_ = make_shared<Framebuffer>( this, 2, math::clamp( g_CVar_vid_msaa.as_i(), 1, 16 ) );
    intermediate_ = make_shared<Framebuffer>( this, 2, 1 );

    g_pointrender = make_unique<PointRenderBuffer>();

    reset( width, height );
  }

  void Renderer::reset( size_t width, size_t height )
  {
    assert( mainbuffer_ && intermediate_ );
    mainbuffer_->recreate( width, height );
    intermediate_->recreate( width, height );
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
      for ( auto& layer : mat->layers_ )
      {
        layer.texture_ = make_shared<Texture>( this,
          layer.image_.width_, layer.image_.height_, layer.image_.format_,
          layer.image_.data_.data(), Texture::ClampEdge, Texture::Mipmapped );
      }
    }
  }

  void Renderer::uploadModelsEnterNode( SceneNode* node )
  {
    if ( node->mesh_ && !node->mesh_->mesh_ )
      node->mesh_->mesh_ = meshes_->createStatic( GL_TRIANGLES, node->mesh_->vertices_, node->mesh_->indices_ );
    for ( auto child : node->children_ )
      uploadModelsEnterNode( child );
  }

  void Renderer::uploadModels()
  {
    vector<SceneNode*> nodes;
    loader_->getFinishedModels( nodes );
    if ( nodes.empty() )
      return;

    console_->printf( Console::srcGfx, "Renderer::uploadModels got %d new models", nodes.size() );

    for ( auto node : nodes )
      uploadModelsEnterNode( node );

    sceneGraph_.insert( nodes.begin(), nodes.end() );
  }

  static bool g_textAdded = false;
  static shared_ptr<DynamicText> g_testText;

  void Renderer::prepare( GameTime time )
  {
    // Upload any new textures. Could this be parallellized?
    uploadTextures();

#ifndef NEKO_NO_SCRIPTING
    meshes_->jsUpdate( director_->renderSync() );
#endif

    // Delete all handles for which our buffer objects were already destroyed
    meshes_->destroyFreed();

    uploadModels();

    // VAOs can and will refer to VBOs and EBOs, and those must have been uploaded by the point at which we try to create the VAO.
    // Thus uploading the VAOs should always come last.
    meshes_->uploadVBOs();
    meshes_->uploadEBOs();
    meshes_->uploadVAOs();

#ifndef NEKO_NO_SCRIPTING
    models_->jsUpdate( director_->renderSync() );
#endif

    if ( !g_testText )
    {
      g_testText = make_shared<DynamicText>( loader_, meshes_, fonts_ );
    }

    if ( g_testText && g_testText->fontLoaded() && !g_textAdded )
    {
      g_textAdded = true;
      g_testText->addText( "nekoengine", vec2( 256.0f, 720.0f - 256.0f + 32.0f ) );
      g_testText->updateTexture( this );
    }
  }

  void Renderer::jsRestart()
  {
#ifndef NEKO_NO_SCRIPTING
    director_->renderSync().resetFromRenderer();
    models_->jsReset();
    meshes_->jsReset();
#endif
  }

  MaterialPtr Renderer::createTextureWithData( size_t width, size_t height, PixelFormat format,
  const void* data, const Texture::Wrapping wrapping, const Texture::Filtering filtering )
  {
    MaterialPtr mat = make_shared<Material>( Material::UnlitSimple );
    MaterialLayer layer;
    layer.image_.format_ = format;
    layer.image_.width_ = (unsigned int)width;
    layer.image_.height_ = (unsigned int)height;
    layer.texture_ = make_shared<Texture>( this,
      layer.image_.width_, layer.image_.height_, layer.image_.format_,
      data, wrapping, filtering );
    mat->layers_.push_back( move( layer ) );
    mat->loaded_ = true;
    return move( mat );
  }

  shaders::Pipeline& Renderer::useMaterial( size_t index )
  {
    GLuint empties[4] = { 0, 0, 0, 0 };
    static bool inited = false;
    if ( !inited )
    {
      for ( unsigned int& it : empties )
        it = builtin_.placeholderTexture_->layers_[0].texture_->handle();
      inited = true;
    }
    if ( index >= materials_.size() || !materials_[index]->uploaded() )
    {
      glBindTextures( 0, 4, empties );
      return shaders_->usePipeline( "mat_unlit" );
    }
    const auto& mat = materials_[index];
    if ( mat->type_ == Material::UnlitSimple )
    {
      glBindTextureUnit( 0, mat->layers_[0].texture_->handle() );
      auto& pipeline = shaders_->usePipeline( "mat_unlit" );
      pipeline.setUniform( "gamma", g_CVar_vid_gamma.as_f() );
      pipeline.setUniform( "tex", 0 );
      return pipeline;
    }
    if ( mat->type_ == Material::WorldPBR )
    {
      GLuint units[4] = {
        mat->layers_[0].texture_->handle(),
        mat->layers_[1].texture_->handle(),
        mat->layers_[2].texture_->handle(),
        mat->layers_[3].texture_->handle()
      };
      glBindTextures( 0, 4, units );
      auto& pipeline = shaders_->usePipeline( "mat_worldpbr" );
      pipeline.setUniform( "gamma", g_CVar_vid_gamma.as_f() );
      pipeline.setUniform( "texAlbedo", 0 );
      pipeline.setUniform( "texHeight", 1 );
      pipeline.setUniform( "texMetallic", 2 );
      pipeline.setUniform( "texNormal", 3 );
      return pipeline;
    }
    return shaders_->usePipeline( "mat_unlit" );
  }

  void Renderer::setCameraUniform( Camera& camera, uniforms::Camera& uniform )
  {
    uniform.position = vec4( camera.position(), 1.0f );
    uniform.projection = camera.projection();
    uniform.view = camera.view();
  }

  void dumpSceneGraph( SceneNode& root, int level = 0 )
  {
    utf8String str;
    for ( int i = 0; i < level; i++ )
      str.append( "  " );
    Locator::console().printf( Console::srcGame,
      "%s<node \"%s\" pos %.2f %.2f %.2f, scale %.2f %.2f %.2f, rot %.2f %.2f %.2f %.2f>", str.c_str(), root.name_.c_str(),
      root.translate_.x, root.translate_.y, root.translate_.z,
      root.scale_.x, root.scale_.y, root.scale_.z,
      root.rotate_.x, root.rotate_.y, root.rotate_.z, root.rotate_.z );
    for ( auto& child : root.children_ )
      dumpSceneGraph( *child, level + 1 );
  }

  void Renderer::sceneDrawEnterNode( SceneNode* node, shaders::Pipeline& pipeline )
  {
    if ( node->mesh_ && node->mesh_->mesh_ )
    {
      node->mesh_->mesh_->begin();
      mat4 model = node->getFullTransform();
      pipeline.setUniform( "model", model );
      node->mesh_->mesh_->draw();
    }
    for ( auto child : node->children_ )
      sceneDrawEnterNode( child, pipeline );
  }

  void Renderer::sceneDraw( GameTime time, Camera& camera )
  {
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );
    glDepthMask( GL_TRUE );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_POLYGON_SMOOTH );

    glEnable( GL_MULTISAMPLE );

    shaders_->world()->time = (float)time;
    setCameraUniform( camera, shaders_->world()->camera );

    for (int i = 0; i < neko::uniforms::c_pointLightCount; i++ )
    {
      shaders_->world()->pointLights[i].dummy = vec4( 0.0f );
    }

    vec3 lightpos[2] = {
      vec3( math::sin( (Real)time * 1.2f ) * 4.0f, 1.0f, math::cos( (Real)time * 1.2f ) * 4.0f ),
      vec3( math::cos( (Real)time * 1.6f ) * 7.5f, math::sin( (Real)time * 1.6f ) * 7.0f, math::sin( (Real)time * 0.25f + 2.0f * 1.6f ) * 7.5f )
    };

    shaders_->world()->pointLights[0].position = vec4( lightpos[0], 1.0f );
    shaders_->world()->pointLights[0].color = vec4( 250.0f, 250.0f, 250.0f, 1.0f );
    shaders_->world()->pointLights[0].dummy = vec4( 1.0f );

    shaders_->world()->pointLights[1].position = vec4( lightpos[1], 1.0f );
    shaders_->world()->pointLights[1].color = vec4( math::sin( (Real)time * 2.0f ) * 50.0f + 50.0f, 0.0f, math::cos( (Real)time * 2.0f ) * 50.0f + 50.0f, 1.0f );
    shaders_->world()->pointLights[1].dummy = vec4( 1.0f );

    shaders_->processing()->ambient = vec4( 0.05f, 0.05f, 0.05f, 1.0f );

    // FIXME ridiculously primitive
    // Also, investigate glMultiDrawArraysIndirect
#ifndef NEKO_NO_SCRIPTING
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

        useMaterial( 0 ).setUniform( "model", mdl );

        mesh->mesh().vao_->draw( GL_TRIANGLES );
      }
    }
#endif

    auto& pl = useMaterial( 2 );
    for ( auto node : sceneGraph_ )
      sceneDrawEnterNode( node, pl );

    /*builtin_.cube_->begin();
    mat4 mdl( 1.0f );
    mdl = glm::scale( mdl, vec3( 3.0f, 3.0f, 3.0f ) );
    useMaterial( 1 ).setUniform( "model", mdl );
    builtin_.cube_->draw();*/

    /*if ( g_CVar_dbg_shownormals.as_b() )
    {
      pl = shaders_->usePipeline( "dbg_showvertexnormals" );
      for ( auto node : sceneGraph_ )
        sceneDrawEnterNode( node, pl );
      //builtin_.cube_->begin();
      //glEnable( GL_LINE_SMOOTH );
      //builtin_.cube_->draw();
    }

    if ( g_CVar_dbg_showtangents.as_b() )
    {
      pl = shaders_->usePipeline( "dbg_showvertextangents" );
      for ( auto node : sceneGraph_ )
        sceneDrawEnterNode( node, pl );
      //builtin_.cube_->begin();
      //glEnable( GL_LINE_SMOOTH );
      //builtin_.cube_->draw();
    }*/

    g_pointrender->buffer().lock();
    auto lightpoints = g_pointrender->buffer().buffer().data();
    lightpoints[0].pos = lightpos[0];
    lightpoints[0].color = shaders_->world()->pointLights[0].color;
    lightpoints[1].pos = lightpos[1];
    lightpoints[1].color = shaders_->world()->pointLights[1].color;
    g_pointrender->buffer().unlock();

    g_pointrender->draw( *shaders_.get(), 2, 0 );

    /*if ( g_testText && g_textAdded )
    {
      g_testText->begin();
      mat4 model( 1.0f );
      model = glm::scale( model, vec3( 1.0f, 1.0f, 1.0f ) );
      model = glm::translate( model, vec3( 1.0f, 1.0f, 0.0f ) );

      auto& pipeline = shaders_->usePipeline( "text3d" );
      pipeline.setUniform( "model", model );
      pipeline.setUniform( "tex", 0 );
      g_testText->draw();
    }*/
  }

  void Renderer::draw( GameTime time, Camera& camera, MyGUI::NekoPlatform* gui )
  {
    if ( !mainbuffer_->available() || !intermediate_->available() )
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
    mainbuffer_->prepare( 0, { 0, 1 } );
    mainbuffer_->begin();
    sceneDraw( time, camera );
    mainbuffer_->end();

    mainbuffer_->blitColorTo( 0, 0, *intermediate_.get() );
    mainbuffer_->blitColorTo( 1, 1, *intermediate_.get() );

    // Framebuffer has been unbound, now draw to the default context, the window.
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    builtin_.screenQuad_->begin();
    auto& pipeline = shaders_->usePipeline( "mainframebuf2d" );
    pipeline.setUniform( "texMain", 0 );
    pipeline.setUniform( "texGBuffer", 1 );
    pipeline.setUniform( "hdr", g_CVar_vid_hdr.as_b() );
    pipeline.setUniform( "gamma", g_CVar_vid_gamma.as_f() );
    pipeline.setUniform( "exposure", g_CVar_vid_exposure.as_f() );

    /*auto pgm = pipeline.getProgramStage( shaders::Shader_Fragment );
    auto pal = gl::glGetUniformLocation( pgm, "palette" );
    vec3 vals[8] = {
      { 1.0f, 0.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f },
      { 0.0f, 0.0f, 1.0f },
      { 1.0f, 1.0f, 0.0f },
      { 0.0f, 1.0f, 1.0f },
      { 1.0f, 0.0f, 1.0f },
      { 0.0f, 0.0f, 0.0f },
      { 0.5f, 0.5f, 0.5f }
    };
    gl::glProgramUniform3fv( pgm, pal, 8, (GLfloat*)vals );
    pipeline.setUniform( "paletteSize", 8 );*/

    glBindTextureUnit( 0, intermediate_->texture( 0 )->handle() );
    glBindTextureUnit( 1, intermediate_->texture( 1 )->handle() );
    builtin_.screenQuad_->draw();

#ifndef NEKO_NO_GUI
    if ( gui )
      gui->getRenderManagerPtr()->drawOneFrame( shaders_.get() );
#endif

    glBindTextureUnit( 0, 0 );
    glBindTextureUnit( 1, 0 );
    glBindVertexArray( builtin_.emptyVAO_ );
  }

  Renderer::~Renderer()
  {
    glBindTextureUnit( 0, 0 );
    glBindVertexArray( builtin_.emptyVAO_ );

    g_pointrender.reset();
    g_testText.reset();

    intermediate_.reset();
    mainbuffer_.reset();

#ifndef NEKO_NO_SCRIPTING
    models_->teardown();
#endif

    meshes_->teardown();

    meshes_->destroyFreed();

    shaders_->shutdown();
    shaders_.reset();

    if ( builtin_.emptyVAO_ )
      glDeleteVertexArrays( 1, &builtin_.emptyVAO_ );
  }

}