#include "pch.h"
#include "loader.h"
#include "utilities.h"
#include "gfx_types.h"
#include "renderer.h"
#include "console.h"
#include "filesystem.h"
#include "spriteanim.h"

#include "lodepng.h"
#include "tinytiffreader.hxx"
#include "tinyexr.h"

namespace neko {

  const string c_loaderThreadName = "nekoLoader";

  const static unicodeString g_prerenderGlyphs = utils::uniFrom( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );

  ThreadedLoader::ThreadedLoader(): thread_( c_loaderThreadName, threadProc, this )
  {
  }

  bool ThreadedLoader::threadProc( platform::Event& running, platform::Event& wantStop, void* argument )
  {
    platform::performanceInitializeLoaderThread();

    auto loader = ( (ThreadedLoader*)argument )->shared_from_this();
    running.set();
    platform::EventVector events = { loader->newTasksEvent_.get(), wantStop.get() };
    while ( true )
    {
      const size_t timeoutValue = 258;
      auto waitRet = platform::waitForEvents( events, 0, false, timeoutValue );
      if ( waitRet == 0 ) // new tasks
      {
        loader->handleNewTasks();
      }
      else if ( waitRet == timeoutValue )
      {
        // Nothing to do lol
        continue;
      }
      else
        break;
    }

    platform::performanceTeardownCurrentThread();
    return true;
  }

  void ThreadedLoader::start()
  {
    thread_.start();
  }

  void ThreadedLoader::stop()
  {
    thread_.stop();
  }

  void ThreadedLoader::addLoadTask( const LoadTaskVector& resources )
  {
    ScopedRWLock lock( &addTaskLock_ );

    newTasks_.reserve( newTasks_.size() + resources.size() );
    newTasks_.insert( newTasks_.end(), resources.begin(), resources.end() );
    newTasksEvent_.set();
  }

  void ThreadedLoader::getFinishedMaterials( MaterialVector& materials )
  {
    if ( !finishedMaterialsEvent_.check() )
      return;

    finishedTasksLock_.lock();
    materials.swap( finishedMaterials_ );
    finishedTasksLock_.unlock();

    finishedMaterials_.clear();
    finishedMaterialsEvent_.reset();
  }

  void ThreadedLoader::getFinishedFonts( FontVector& fonts )
  {
    if ( !finishedFontsEvent_.check() )
      return;

    finishedTasksLock_.lock();
    fonts.swap( finishedFonts_ );
    finishedTasksLock_.unlock();

    finishedFonts_.clear();
    finishedFontsEvent_.reset();
  }

  void ThreadedLoader::getFinishedModels( vector<MeshNodePtr>& models )
  {
    if ( !finishedModelsEvent_.check() )
      return;

    finishedTasksLock_.lock();
    models.swap( finishedModels_ );
    finishedTasksLock_.unlock();

    finishedModels_.clear();
    finishedModelsEvent_.reset();
  }

  void ThreadedLoader::getFinishedAnimations( AnimationVector& animations )
  {
    if ( !finishedAnimationsEvent_.check() )
      return;

    finishedTasksLock_.lock();
    animations.swap( finishedAnimations_ );
    finishedTasksLock_.unlock();

    finishedAnimations_.clear();
    finishedAnimationsEvent_.reset();
  }

  void ThreadedLoader::getFinishedSpritesheets( SpriteAnimationSetDefinitionVector& sheets )
  {
    if ( !finishedSpritesheetsEvent_.check() )
      return;

    finishedTasksLock_.lock();
    sheets.swap( finishedSpritesheets_ );
    finishedTasksLock_.unlock();

    finishedSpritesheets_.clear();
    finishedSpritesheetsEvent_.reset();
  }

  using namespace gl;

  void ThreadedLoader::loadFontFace( LoadTask::FontfaceLoad& task )
  {
    vector<uint8_t> input;
    Locator::fileSystem().openFile( Dir_Fonts, task.path_ )->readFullVector( input );
    auto face = task.font_->loadFace( input, 0 );
    for ( const auto& spec : task.specs_ )
    {
      face->loadStyle( spec.rendering, spec.size, spec.thickness, g_prerenderGlyphs );
    }

    Locator::console().printf( srcLoader, "Loaded font %s (%s) with %i sizes", task.font_->name().c_str(),
      task.path_.c_str(), task.specs_.size() );

    finishedTasksLock_.lock();
    finishedFonts_.push_back( task.font_ );
    finishedTasksLock_.unlock();
  }

  void ThreadedLoader::loadModel( LoadTask::ModelLoad& task )
  {
    auto path = task.path_;

    auto filename = utils::extractFilename( path );
    auto filepath = utils::extractFilepath( R"(assets\meshes\)" + path );
    auto ext = utils::extractExtension( path );

    if ( ext == L"gltf" )
    {
      vector<uint8_t> input;
      Locator::fileSystem().openFile( Dir_Meshes, path )->readFullVector( input );
      loaders::loadGLTFModel(
        input, platform::wideToUtf8( filename ), platform::wideToUtf8( filepath ), task.node_ );
    }
    else
      NEKO_EXCEPT( "Unsupported mesh format in load model task" );

    finishedTasksLock_.lock();
    finishedModels_.push_back( task.node_ );
    finishedTasksLock_.unlock();
  }

  Pixmap ThreadedLoader::loadTexture( const utf8String& path )
  {
    vector<uint8_t> input;
    Locator::fileSystem().openFile( Dir_Textures, path )->readFullVector( input );

    auto ext = utils::extractExtension( path );

    if ( ext == L"png" )
    {
      auto pmp = Pixmap::fromPNG( input );
      Locator::console().printf( srcLoader, "Loaded PNG texture %s, %ix%i, rgba8", path.c_str(), pmp.width(), pmp.height() );
      return pmp;
    }
    else if ( ext == L"exr" )
    {
      auto pmp = Pixmap::fromEXR( input );
      Locator::console().printf(
        srcLoader, "Loaded EXR texture %s, %ix%i, rgba32f", path.c_str(), pmp.width(), pmp.height() );
      return pmp;
    }
    else if ( ext == L"tif" )
    {
      auto pmp = Pixmap::fromTIFF( path );
      Locator::console().printf( srcLoader, "Loaded TIFF texture %s, %ix%i", path.c_str(), pmp.width(), pmp.height() );
      return pmp;
    }
    else
      NEKO_EXCEPT( "Unsupport image format in load texture task" );

    return Pixmap( PixFmtColorRGBA8 );
  }

  void ThreadedLoader::loadMaterial( LoadTask::TextureLoad& task )
  {
    for ( const auto& target : task.paths_ )
    {
      MaterialLayer layer( loadTexture( target ) );
      task.material_->wantWrapping_ = Texture::Repeat;
      task.material_->layers_.push_back( move( layer ) );
    }

    task.material_->loaded_ = true;

    finishedTasksLock_.lock();
    finishedMaterials_.push_back( task.material_ );
    finishedTasksLock_.unlock();
  }

  void ThreadedLoader::loadSpritesheet( LoadTask::SpritesheetLoad& task )
  {
    map<utf8String, PixmapPtr> textures;
    for ( const auto& [key, it] : task.def_->entries_ )
    {
      if ( it->material_ )
        continue;

      if ( !textures.contains( it->sheetName_ ) )
      {
        textures[it->sheetName_] = make_shared<Pixmap>( loadTexture( it->sheetName_ ) );
        textures[it->sheetName_]->flipVertical();
      }

      auto sheet = textures.at( it->sheetName_ );
      Pixmap cut( *sheet, it->sheetPos_.x, it->sheetPos_.y, it->definition_->frameCount() * it->definition_->width(),
        it->definition_->height() );
      auto matname = task.def_->name_ + "_" + it->name_;
      cut.writePNG( matname + ".png" );
    }
    //const auto& adef = animdefs_.at( def.defName_ );
    //def.material_ = renderer_->createTextureWithData( matname, adef.width(), adef.height(), adef.frameCount(), cut.format(),
    //  cut.data().data(), Texture::ClampBorder, Texture::Nearest );
  }

  // Context: Worker thread
  void ThreadedLoader::handleNewTasks()
  {
    LoadTaskVector newTasks;

    addTaskLock_.lock();
    newTasks.swap( newTasks_ );
    newTasksEvent_.reset();
    addTaskLock_.unlock();

    for ( auto& task : newTasks )
    {
      if ( task.type_ == LoadTask::Load_Texture )
      {
        loadMaterial( task.textureLoad );
      }
      else if ( task.type_ == LoadTask::Load_Fontface )
      {
        loadFontFace( task.fontfaceLoad );
      }
      else if ( task.type_ == LoadTask::Load_Model )
      {
        loadModel( task.modelLoad );
      }
      else if ( task.type_ == LoadTask::Load_Spritesheet )
      {
        loadSpritesheet( task.spriteLoad );
      }
    }

    if ( !finishedMaterials_.empty() )
      finishedMaterialsEvent_.set();

    if ( !finishedFonts_.empty() )
      finishedFontsEvent_.set();

    if ( !finishedModels_.empty() )
      finishedModelsEvent_.set();

    if ( !finishedAnimations_.empty() )
      finishedAnimationsEvent_.set();

    if ( !finishedSpritesheets_.empty() )
      finishedSpritesheetsEvent_.set();
  }

  ThreadedLoader::~ThreadedLoader()
  {
  }

}