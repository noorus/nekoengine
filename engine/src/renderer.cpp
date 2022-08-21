#include "pch.h"
#include "gfx_types.h"
#include "renderer.h"
#include "meshmanager.h"
#include "shaders.h"
#include "camera.h"
#include "loader.h"
#include "engine.h"
#include "font.h"
#include "lodepng.h"
#include "gfx.h"
#include "nekosimd.h"
#include "particles.h"
#include "math_aabb.h"
#include "text.h"
#include "filesystem.h"

namespace neko {

  using namespace gl;

  NEKO_DECLARE_CONVAR( vid_msaa, "Main buffer multisample antialiasing multiplier.", 8 );
  NEKO_DECLARE_CONVAR( dbg_shownormals, "Whether to visualize vertex normals with lines.", false );
  NEKO_DECLARE_CONVAR( dbg_showtangents, "Whether to visualize vertex tangents with lines.", false );
  NEKO_DECLARE_CONVAR( dbg_wireframe, "Whether to render in wireframe mode.", false );
  NEKO_DECLARE_CONVAR( dbg_showdepth, "Whether to visualize the depth buffer.", false );
  NEKO_DECLARE_CONVAR( vid_hdr, "Toggle HDR processing.", true );
  NEKO_DECLARE_CONVAR( vid_gamma, "Screen gamma target.", 2.2f );
  NEKO_DECLARE_CONVAR( vid_exposure, "Testing.", 1.0f );
  NEKO_DECLARE_CONVAR( gl_dump, "Dump OpenGL extensions on startup.", false );

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

  }

  const auto c_bufferFormat = PixFmtColorRGBA16f;

  void glStartupFetchAndCheck( GLInformation& info, Console& console, bool dumpExts )
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

    if ( dumpExts )
    {
      struct GLVendor
      {
        utf8String prefix;
        utf8String name;
        utf8String extensions;
      };

      GLVendor vendors[5] = {
        { "_ARB", "ARB", "" },
        { "_NV", "NVIDIA", "" },
        { "_AMD", "AMD", "" },
        { "_INTEL", "Intel", "" },
        { "_", "Other", "" }
      };

      const auto extcount = glvGetI32NoThrow( GL_NUM_EXTENSIONS );
      for ( int32_t i = 0; i < extcount; ++i )
      {
        const auto extb = glGetStringi( GL_EXTENSIONS, i );
        if ( !extb )
          continue;
        const string ext( reinterpret_cast<const char*>( extb ) );
        for ( auto& vendor : vendors )
          if ( ext.find( vendor.prefix ) != ext.npos )
          {
            vendor.extensions.append( vendor.extensions.empty() ? ext : ( " " + ext ) );
            break;
          }
      }

      for ( auto& vendor : vendors )
        if ( !vendor.extensions.empty() )
          console.printf( Console::srcGfx, "%s Extensions: %s", vendor.name.c_str(), vendor.extensions.c_str() );
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

    // Maximum anisotropic filtering level
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
    glStartupFetchAndCheck( info_, *console_.get(), g_CVar_gl_dump.as_b() );

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

    materials_ = make_shared<MaterialManager>( this, loader_ );
    meshes_ = make_shared<MeshManager>( console_ );
#ifndef NEKO_NO_SCRIPTING
    models_ = make_shared<ModelManager>( console_ );
    texts_ = make_shared<TextManager>( fonts_, console_ );
#endif

    glCreateVertexArrays( 1, &builtin_.emptyVAO_ );

    builtin_.placeholderTexture_ = createTextureWithData( "int_placeholder", 2, 2, PixFmtColorRGBA8,
      (const void*)BuiltinData::placeholderImage2x2.data() );

    builtin_.screenQuad_ = meshes_->createStatic( GL_TRIANGLES, BuiltinData::screenQuad2D, BuiltinData::quadIndices );

    auto unitSphere = Locator::meshGenerator().makeSphere( 1.0f, vec2u( 32, 32 ) );
    builtin_.unitSphere_ = meshes_->createStatic( GL_TRIANGLE_STRIP, unitSphere.first, unitSphere.second );

    auto skybox = Locator::meshGenerator().makeBox( vec3( 1.0f ), vec2u( 1 ), true, vec4( 1.0f ) );
    builtin_.skybox_ = meshes_->createStatic( GL_TRIANGLES, skybox.first, skybox.second );
  }

  TexturePtr Renderer::loadPNGTexture( const utf8String& filepath, Texture::Wrapping wrapping, Texture::Filtering filtering )
  {
    vector<uint8_t> input, output;
    Locator::fileSystem().openFileAbsolute( filepath )->readFullVector( input );
    unsigned int w, h;
    if ( lodepng::decode( output, w, h, input.data(), input.size(), LCT_RGBA, 8 ) != 0 )
      NEKO_EXCEPT( "Lodepng image load failed" );
    return make_shared<Texture>( this, w, h, PixFmtColorR8, output.data(), wrapping, filtering );
  }

  void Renderer::initialize( size_t width, size_t height )
  {
    resolution_ = vec2( static_cast<Real>( width ), static_cast<Real>( height ) );

    materials_->loadFile( R"(materials.json)" );

    fonts_->initializeRender( this );
    fonts_->loadFile( R"(fonts.json)" );

    loader_->addLoadTask( { LoadTask( new SceneNode(), R"(dbg_normaltestblock.gltf)" ) } );

    ctx_.fboMain_ = make_unique<Framebuffer>( this, 2, c_bufferFormat, true, 0 );

    reset( width, height );
  }

  void Renderer::reset( size_t width, size_t height )
  {
    resolution_ = vec2( static_cast<Real>( width ), static_cast<Real>( height ) );
    ctx_.fboMain_->recreate( width, height );
  }

  void Renderer::uploadTextures()
  {
    MaterialVector mats;
    loader_->getFinishedMaterials( mats );
    if ( !mats.empty() )
      console_->printf( Console::srcGfx, "Renderer::uploadTextures got %d new materials", mats.size() );
    for ( auto& mat : mats )
    {
      if ( !mat->loaded() )
        continue;
      for ( auto& layer : mat->layers_ )
      {
        assert( layer.hasHostCopy() );
        layer.texture_ = make_shared<Texture>( this,
          layer.image_.width_, layer.image_.height_,
          layer.image_.format_, layer.image_.data_.data(),
          mat->wantWrapping_, mat->wantFiltering_ );
        layer.deleteHostCopy();
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

    fonts_->prepareRender();

    // VAOs can and will refer to VBOs and EBOs, and those must have been uploaded by the point at which we try to create the VAO.
    // Thus uploading the VAOs should always come last.
    meshes_->uploadVBOs();
    meshes_->uploadEBOs();
    meshes_->uploadVAOs();

#ifndef NEKO_NO_SCRIPTING
    models_->jsUpdate( director_->renderSync() );
    texts_->jsUpdate( director_->renderSync() );
#endif
  }

  void Renderer::jsRestart()
  {
#ifndef NEKO_NO_SCRIPTING
    director_->renderSync().resetFromRenderer();
    texts_->jsReset();
    models_->jsReset();
    meshes_->jsReset();
#endif
  }

  MaterialPtr Renderer::createTextureWithData( const utf8String& name, size_t width, size_t height, PixelFormat format,
  const void* data, const Texture::Wrapping wrapping, const Texture::Filtering filtering )
  {
    return materials_->createTextureWithData( name, width, height, format, data, wrapping, filtering );
  }

  shaders::Pipeline& Renderer::useMaterial( const utf8String& name )
  {
    GLuint empties[4] = { 0, 0, 0, 0 };
    static bool inited = false;
    if ( !inited )
    {
      for ( unsigned int& it : empties )
        it = builtin_.placeholderTexture_->layers_[0].texture_->handle();
      inited = true;
    }
    auto material = materials_->get( name );
    if ( !material || !material->uploaded() )
    {
      glBindTextures( 0, 4, empties );
      return shaders_->usePipeline( "mat_unlit" );
    }
    if ( material->type_ == Material::UnlitSimple )
    {
      GLuint units[4] = {
        material->layers_[0].texture_->handle(),
        material->layers_[0].texture_->handle(),
        material->layers_[0].texture_->handle(),
        material->layers_[0].texture_->handle() };
      glBindTextures( 0, 4, units );
      auto& pipeline = shaders_->usePipeline( "mat_unlit" );
      pipeline.setUniform( "tex", 0 );
      return pipeline;
    }
    if ( material->type_ == Material::WorldParticle )
    {
    }
    return shaders_->usePipeline( "mat_unlit" );
  }

  void Renderer::setCameraUniforms( Camera& camera, uniforms::Camera& uniform )
  {
    uniform.position = vec4( camera.position(), 1.0f );
    uniform.projection = camera.projection();
    uniform.view = camera.view();
    uniform.nearDist = camera.near();
    uniform.farDist = camera.far();
    uniform.exposure = camera.exposure();
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

  void Renderer::setUserData( uint64_t id, const utf8String name, rainet::Image& image )
  {
    userData_.name_ = name;
    userData_.image_ = createTextureWithData( "rainet_avatar_" + id, image.width_, image.height_, PixFmtColorRGBA8, image.buffer_.data() );
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

  static unique_ptr<SakuraSystem> g_sakura;

  void setGLDrawState( bool depthtest, bool depthwrite, bool facecull )
  {
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_STENCIL_TEST );

    glDepthFunc( GL_LESS );

    if ( depthtest )
      glEnable( GL_DEPTH_TEST );
    else
      glDisable( GL_DEPTH_TEST );
    glDepthMask( depthwrite ? GL_TRUE : GL_FALSE );

    glPolygonMode( GL_FRONT_AND_BACK, g_CVar_dbg_wireframe.as_b() ? GL_LINE : GL_FILL );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_POLYGON_SMOOTH );
    glDisable( GL_MULTISAMPLE );

    if ( facecull )
    {
      glEnable( GL_CULL_FACE );
      glCullFace( GL_BACK );
      glFrontFace( GL_CCW );
    }
    else
      glDisable( GL_CULL_FACE );
  }

  void Renderer::sceneDraw( GameTime time, GameTime delta, Camera& camera )
  {
    camera.exposure( g_CVar_vid_exposure.as_f() );

    {
      auto world = shaders_->world()->lock().data();
      world->time = (float)time;
      setCameraUniforms( camera, world->camera );
      for ( auto& pointLight : world->pointLights )
        pointLight.dummy = vec4( 0.0f );
      shaders_->world()->unlock();
    }

    setGLDrawState( true, true, true );

    auto fn_drawModels = [&]( shaders::Pipeline& pipeline ) -> void
    {
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

          pipeline.setUniform( "model", mdl );

          mesh->mesh().vao_->draw( GL_TRIANGLES );
        }
      }
#endif
    };

    if ( false )
    {
      auto& pipeline = useMaterial( "demo_uvtest" );
      fn_drawModels( pipeline );
    }

    if ( false )
    {
      auto& pipeline = useMaterial( "demo_uvtest" );
      for ( auto node : sceneGraph_ )
        sceneDrawEnterNode( node, pipeline );
    }

    glEnable( GL_LINE_SMOOTH );

    if ( g_CVar_dbg_shownormals.as_b() )
    {
      auto& pipeline = shaders_->usePipeline( "dbg_showvertexnormals" );
      fn_drawModels( pipeline );
      for ( auto node : sceneGraph_ )
        sceneDrawEnterNode( node, pipeline );
    }

    if ( g_CVar_dbg_showtangents.as_b() )
    {
      auto& pipeline = shaders_->usePipeline( "dbg_showvertextangents" );
      fn_drawModels( pipeline );
      for ( auto node : sceneGraph_ )
        sceneDrawEnterNode( node, pipeline );
    }

    glDisable( GL_LINE_SMOOTH );

    {
      setGLDrawState( true, true, false );
      if ( !g_sakura )
        g_sakura = make_unique<SakuraSystem>( aabb( vec3( -8.0f, -2.0f, -8.0f ), vec3( 8.0f, 10.0f, 8.0f ) ) );

      g_sakura->update( delta );
      const auto partmat = materials_->get( "demo_sakura" );
      g_sakura->draw( *shaders_.get(), *partmat );
    }

    {
      setGLDrawState( true, true, true );
      glDepthFunc( GL_LEQUAL );
      auto& pipeline = shaders_->usePipeline( "mat_skybox" );
      pipeline.setUniform( "tex", 0 );
      auto mat = materials_->get( "demo_skybox" );
      if ( mat && mat->uploaded() )
      {
        const GLuint hndl = mat->layers_[0].texture_->handle();
        glBindTextures( 0, 1, &hndl );
        builtin_.skybox_->drawOnce( pipeline, vec3( 0.0f ) );
      }
      glDepthFunc( GL_LESS );
    }

    if ( false )
    {
      setGLDrawState( false, false, true );
      auto& pipeline = shaders_->usePipeline( "mat_screentone" );
      builtin_.screenQuad_->begin();
      builtin_.screenQuad_->draw();
    }
  }

  void Renderer::implClearAndPrepare()
  {
    // set the clear color
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    // enable depth writes or there is no depth clearing
    glDepthMask( TRUE );
    // reset depth func
    glDepthFunc( GL_LESS );
    // finally, clear buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  }

  static FontPtr g_text;

  void Renderer::draw( GameTime time, GameTime delta, Camera& camera, MyGUI::NekoPlatform* gui )
  {
    // check that the drawcontext is ready (fbo's available etc)
    if ( !ctx_.ready() )
      return;

    // Default to empty VAO, since not having a bound VAO is illegal as per 4.5 spec
    glBindVertexArray( builtin_.emptyVAO_ );

    {
      auto processing = shaders_->processing()->lock().data();
      processing->ambient = vec4( 0.04f, 0.04f, 0.04f, 1.0f );
      processing->gamma = g_CVar_vid_gamma.as_f();
      processing->resolution = resolution_;
      processing->textproj = glm::ortho( 0.0f, resolution_.x, resolution_.y, 0.0f );
      shaders_->processing()->unlock();
    }

    ctx_.fboMain_->prepare( 0, { 0, 1 } );
    ctx_.fboMain_->begin();
    implClearAndPrepare(); // 1 - clear the main fbo
    sceneDraw( time, delta, camera );
    ctx_.fboMain_->end();

    implClearAndPrepare(); // 2 - clear the window

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glDisable( GL_MULTISAMPLE );
    glDisable( GL_STENCIL_TEST );

    // Smoothing can generate sub-fragments and cause visible ridges between triangles.
    // Use a framebuffer for AA instead.
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_POLYGON_SMOOTH );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // Early out - if dbg_showdepth is enabled, just show the main depth buffer
    if ( g_CVar_dbg_showdepth.as_b() )
    {
      setGLDrawState( false, false, true );
      auto& pipeline = shaders_->usePipeline( "dbg_depthvis2d" );
      pipeline.setUniform( "tex", 0 );
      GLuint handle = ctx_.fboMain_->depth()->handle();
      glBindTextures( 0, 1, &handle );
      builtin_.screenQuad_->begin();
      builtin_.screenQuad_->draw();
      return;
    }

    // Draw the main FBO, including HDR mergedown with tonemapping
    {
      setGLDrawState( false, false, true );
      auto& pipeline = shaders_->usePipeline( "mainframebuf2d" );
      pipeline.setUniform( "tex", 0 );
      const GLuint hndl = ctx_.fboMain_->texture( 0 )->handle();
      glBindTextures( 0, 1, &hndl );
      builtin_.screenQuad_->begin();
      builtin_.screenQuad_->draw();
    }

    for ( auto& text : texts_->texts() )
    {
      auto& txt = text.second->text();
      if ( !txt.impl_ )
        txt.impl_ = fonts_->createText( "demo_font", 0 );
      txt.impl_->content( txt.content_ );
      txt.impl_->setScale( txt.scale_->v() );
      txt.impl_->setRotation( txt.rotate_->q() );
      txt.impl_->setTranslation( txt.translate_->v() );
      txt.impl_->update( this );
      txt.impl_->draw( this );
    }

    // below here be debug shit
    #if 0
    if ( userData_.image_ && userData_.image_->uploaded() )
    {
      setGLDrawState( false, false, true );
      auto& pipeline = shaders_->usePipeline( "passthrough2d" );
      pipeline.setUniform( "tex", 0 );
      GLuint handle = userData_.image_->textureHandle( 0 );
      glBindTextures( 0, 1, &handle );
      builtin_.screenQuad_->begin();
      builtin_.screenQuad_->draw();
    }
#endif

    fonts_->draw();

#ifndef NEKO_NO_GUI
    if ( gui )
      gui->getRenderManagerPtr()->drawOneFrame( shaders_.get() );
#endif

    glBindTextureUnit( 0, 0 );
    glBindTextureUnit( 1, 0 );
    glBindVertexArray( builtin_.emptyVAO_ );
  }

  void Renderer::shutdown()
  {
    fonts_->shutdownRender();
  }

  Renderer::~Renderer()
  {
    shutdown();
#ifndef INTEL_SUCKS
    glBindTextureUnit( 0, 0 );
    glBindVertexArray( builtin_.emptyVAO_ );

    g_sakura.reset();

    ctx_.fboMain_.reset();

#ifndef NEKO_NO_SCRIPTING
    texts_->teardown();
    models_->teardown();
#endif

    meshes_->teardown();

    meshes_->destroyFreed();

    materials_.reset();

    shaders_->shutdown();
    shaders_.reset();

    if ( builtin_.emptyVAO_ )
      glDeleteVertexArrays( 1, &builtin_.emptyVAO_ );
#else
    ExitProcess( 0 );
#endif
  }

}