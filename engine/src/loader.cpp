#include "stdafx.h"
#include "loader.h"
#include "utilities.h"
#include "lodepng.h"
#include "renderer.h"
#include "console.h"

namespace neko {

  const string c_loaderThreadName = "nekoLoader";

  ThreadedLoader::ThreadedLoader(): thread_( c_loaderThreadName, threadProc, this ), fbxmgr_( nullptr ), fbxio_( nullptr )
  {
    fbxmgr_ = FbxManager::Create();
    fbxio_ = FbxIOSettings::Create( fbxmgr_, IOSROOT );
    fbxmgr_->SetIOSettings( fbxio_ );
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

  FbxString GetAttributeTypeName( FbxNodeAttribute::EType type )
  {
    switch ( type )
    {
      case FbxNodeAttribute::eUnknown: return "unidentified";
      case FbxNodeAttribute::eNull: return "null";
      case FbxNodeAttribute::eMarker: return "marker";
      case FbxNodeAttribute::eSkeleton: return "skeleton";
      case FbxNodeAttribute::eMesh: return "mesh";
      case FbxNodeAttribute::eNurbs: return "nurbs";
      case FbxNodeAttribute::ePatch: return "patch";
      case FbxNodeAttribute::eCamera: return "camera";
      case FbxNodeAttribute::eCameraStereo: return "stereo";
      case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
      case FbxNodeAttribute::eLight: return "light";
      case FbxNodeAttribute::eOpticalReference: return "optical reference";
      case FbxNodeAttribute::eOpticalMarker: return "marker";
      case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
      case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
      case FbxNodeAttribute::eBoundary: return "boundary";
      case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
      case FbxNodeAttribute::eShape: return "shape";
      case FbxNodeAttribute::eLODGroup: return "lodgroup";
      case FbxNodeAttribute::eSubDiv: return "subdiv";
      default: return "unknown";
    }
  }

  void PrintAttribute( FbxNodeAttribute* pAttribute )
  {
    if ( !pAttribute )
      return;
    FbxString typeName = GetAttributeTypeName( pAttribute->GetAttributeType() );
    FbxString attrName = pAttribute->GetName();
    Locator::console().printf( Console::srcEngine, "<attribute type='%s' name='%s'/>", typeName.Buffer(), attrName.Buffer() );
  }

  void PrintNode( FbxNode* pNode )
  {
    const char* nodeName = pNode->GetName();
    FbxDouble3 translation = pNode->LclTranslation.Get();
    FbxDouble3 rotation = pNode->LclRotation.Get();
    FbxDouble3 scaling = pNode->LclScaling.Get();
    Locator::console().printf( Console::srcEngine,
      "<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>",
      nodeName,
      translation[0], translation[1], translation[2],
      rotation[0], rotation[1], rotation[2],
      scaling[0], scaling[1], scaling[2]
    );
    for ( int i = 0; i < pNode->GetNodeAttributeCount(); i++ )
      PrintAttribute( pNode->GetNodeAttributeByIndex( i ) );
    for ( int j = 0; j < pNode->GetChildCount(); j++ )
      PrintNode( pNode->GetChild( j ) );
    Locator::console().printf( Console::srcEngine, "</node>" );
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
        auto importer = FbxImporter::Create( fbxmgr_, "" );
        if ( importer->Initialize( task.modelLoad.path_.c_str(), -1, fbxmgr_->GetIOSettings() ) )
        {
          auto scene = FbxScene::Create( fbxmgr_, "loaderScene" );
          importer->Import( scene );
          auto root = scene->GetRootNode();
          //if ( root )
          //  PrintNode( root );
        }
        importer->Destroy( true );
      }
    }

    if ( !finishedMaterials_.empty() )
      finishedMaterialsEvent_.set();

    if ( !finishedFonts_.empty() )
      finishedFontsEvent_.set();
  }

  ThreadedLoader::~ThreadedLoader()
  {
    if ( fbxio_ )
      fbxio_->Destroy( true );
    if ( fbxmgr_ )
      fbxmgr_->Destroy();
  }

}