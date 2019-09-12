#include "stdafx.h"
#include "loader.h"
#include "utilities.h"
#include "lodepng.h"
#include "renderer.h"

namespace neko {

  const string c_loaderThreadName = "nekoLoader";

  ThreadedLoader::ThreadedLoader():
    thread_( c_loaderThreadName, threadProc, this )
  {
    OutputDebugStringA( "ThreadedLoader::ThreadedLoader()\r\n" );
  }

  bool ThreadedLoader::threadProc( platform::Event& running, platform::Event& wantStop, void* argument )
  {
    auto loader = ( (ThreadedLoader*)argument )->shared_from_this();
    running.set();
    platform::EventVector events = { loader->newTasksEvent_.get(), wantStop.get() };
    while ( true )
    {
      const size_t timeoutValue = 258;
      auto waitRet = platform::waitForEvents( events, 1000, false, timeoutValue );
      if ( waitRet == 0 ) // new tasks
      {
        loader->handleNewTasks();
        OutputDebugStringA( "ThreadedLoader::threadProc() new tasks!\r\n" );
      }
      else if ( waitRet == timeoutValue )
      {
        // Nothing to do lol
        continue;
      }
      else
        break;
    }
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
    if ( !finishedTasksEvent_.check() )
      return;

    finishedTasksLock_.lock();
    materials.swap( finishedMaterials_ );
    finishedTasksLock_.unlock();

    finishedMaterials_.clear();
    finishedTasksEvent_.reset();
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
        char asd[256];
        sprintf_s( asd, 256, "ThreadedLoader::handleNewTasks processing texture from %s\r\n", task.textureLoad.path_.c_str() );
        OutputDebugStringA( asd );
        vector<uint8_t> input;
        unsigned int width, height;
        platform::FileReader( task.textureLoad.path_ ).readFullVector( input );
        if ( lodepng::decode( task.textureLoad.material_->image_.data_, width, height, input.data(), input.size(), LCT_RGBA, 8 ) == 0 )
        {
          task.textureLoad.material_->image_.width_ = width;
          task.textureLoad.material_->image_.height_ = height;
          task.textureLoad.material_->image_.format_ = PixFmtColorRGBA8;
          task.textureLoad.material_->loaded_ = true;
          finishedTasksLock_.lock();
          finishedMaterials_.push_back( task.textureLoad.material_ );
          finishedTasksLock_.unlock();
        }
        else
        {
          //task.textureLoad.material_->
        }
      }
      else if ( task.type_ == LoadTask::Load_Fontface )
      {
        vector<uint8_t> input;
        platform::FileReader( task.fontfaceLoad.path_ ).readFullVector( input );
        task.fontfaceLoad.font_ = move( task.fontfaceLoad.manager_->loadFace( input.data(), input.size(), task.fontfaceLoad.size_ * 64, 72, 72 ) );
      }
    }

    if ( !finishedMaterials_.empty() )
      finishedTasksEvent_.set();
  }

  ThreadedLoader::~ThreadedLoader()
  {
    OutputDebugStringA( "ThreadedLoader::~ThreadedLoader()\r\n" );
  }

}