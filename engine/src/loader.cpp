#include "stdafx.h"
#include "loader.h"
#include "utilities.h"
#include "lodepng.h"
#include "gfx_types.h"
#include "renderer.h"
#include "console.h"

namespace neko {

  const string c_loaderThreadName = "nekoLoader";

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

  void ThreadedLoader::getFinishedModels( vector<SceneNode*>& models )
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
        for ( const auto& path : task.textureLoad.paths_ )
        {
          vector<uint8_t> input;
          unsigned int width, height;
          platform::FileReader( path ).readFullVector( input );
          MaterialLayer layer;
          if ( lodepng::decode( layer.image_.data_, width, height, input.data(), input.size(), LCT_RGBA, 8 ) == 0 )
          {
            layer.image_.width_ = width;
            layer.image_.height_ = height;
            layer.image_.format_ = PixFmtColorRGBA8;
            task.textureLoad.material_->layers_.push_back( move( layer ) );
          }
          else
          {
            NEKO_EXCEPT( "Unsupport image format in load texture task" );
          }
        }
        task.textureLoad.material_->loaded_ = true;
        finishedTasksLock_.lock();
        finishedMaterials_.push_back( task.textureLoad.material_ );
        finishedTasksLock_.unlock();
      }
      else if ( task.type_ == LoadTask::Load_Fontface )
      {
        vector<uint8_t> input;
        platform::FileReader( task.fontfaceLoad.path_ ).readFullVector( input );
        task.fontfaceLoad.font_->manager_->loadFont( task.fontfaceLoad.font_, task.fontfaceLoad.specs_, input );
        finishedTasksLock_.lock();
        finishedFonts_.push_back( task.fontfaceLoad.font_ );
        finishedTasksLock_.unlock();
      }
      else if ( task.type_ == LoadTask::Load_Model )
      {
        vector<uint8_t> input;
        //platform::FileReader( task.modelLoad.path_ ).readFullVector( input );
        fbxLoader_.loadFBXScene( input, task.modelLoad.path_, task.modelLoad.node_ );
        finishedTasksLock_.lock();
        finishedModels_.push_back( task.modelLoad.node_ );
        finishedTasksLock_.unlock();
      }
      else if ( task.type_ == LoadTask::Load_Animation )
      {
        vector<uint8_t> input;
        platform::FileReader( task.animationLoad.path_ ).readFullVector( input );
        loaders::loadUnityAnimation( input, task.animationLoad.animation_ );
        finishedTasksLock_.lock();
        finishedAnimations_.push_back( task.animationLoad.animation_ );
        finishedTasksLock_.unlock();
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