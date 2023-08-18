#include "pch.h"
#include "loader.h"
#include "utilities.h"
#include "gfx_types.h"
#include "renderer.h"
#include "console.h"
#include "filesystem.h"

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

  void ThreadedLoader::loadTexture( LoadTask::TextureLoad& task )
  {
    for ( const auto& target : task.paths_ )
    {
      vector<uint8_t> input;
      Locator::fileSystem().openFile( Dir_Textures, target )->readFullVector( input );
      vector<uint8_t> rawData;
      task.material_->wantWrapping_ = Texture::Repeat;
      auto ext = utils::extractExtension( target );
      if ( ext == L"png" )
      {
        MaterialLayer layer( Pixmap::fromPNG( input ) );
        Locator::console().printf( srcLoader, "Loaded PNG texture %s, %ix%i, rgba8", target.c_str(), layer.width(), layer.height() );
        task.material_->layers_.push_back( move( layer ) );
      }
      else if ( ext == L"exr" )
      {
        MaterialLayer layer( Pixmap::fromEXR( input ) );
        Locator::console().printf(
          srcLoader, "Loaded EXR texture %s, %ix%i, rgba32f", target.c_str(), layer.width(), layer.height() );
        task.material_->layers_.push_back( move( layer ) );
      }
      else if ( ext == L"tif" )
      {
        MaterialLayer layer( Pixmap::fromTIFF( target ) );
        Locator::console().printf(
          srcLoader, "Loaded TIFF texture %s, %ix%i", target.c_str(), layer.width(), layer.height() );
        task.material_->layers_.push_back( move( layer ) );
      }
      else
        NEKO_EXCEPT( "Unsupport image format in load texture task" );
    }

    task.material_->loaded_ = true;

    finishedTasksLock_.lock();
    finishedMaterials_.push_back( task.material_ );
    finishedTasksLock_.unlock();
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
        loadTexture( task.textureLoad );
      }
      else if ( task.type_ == LoadTask::Load_Fontface )
      {
        loadFontFace( task.fontfaceLoad );
      }
      else if ( task.type_ == LoadTask::Load_Model )
      {
        loadModel( task.modelLoad );
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
  }

  ThreadedLoader::~ThreadedLoader()
  {
  }

}