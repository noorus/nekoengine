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

  void ThreadedLoader::getFinishedModels( vector<ModelLoadOutput>& models )
  {
    if ( !finishedModelsEvent_.check() )
      return;

    finishedTasksLock_.lock();
    models.swap( finishedModels_ );
    finishedTasksLock_.unlock();

    finishedModels_.clear();
    finishedModelsEvent_.reset();
  }

  /*void generateTangents( vector<Vertex3D>& verts, const vector<GLuint>& indices )
  {
    for ( GLuint l = 0; l < indices.size(); ++l )
      verts[indices[l]].tangent = vec4( 0.0f );

    int inconsistents = 0;
    for ( GLuint l = 0; l < indices.size(); ++l )
    {
      GLuint i = indices[l];
      GLuint j = indices[( l + 1 ) % 3 + l / 3 * 3];
      GLuint k = indices[( l + 2 ) % 3 + l / 3 * 3];
      vec3 n = verts[i].normal;
      vec3 v1 = verts[j].position - verts[i].position, v2 = verts[k].position - verts[i].position;
      vec2 t1 = verts[j].texcoord - verts[i].texcoord, t2 = verts[k].texcoord - verts[i].texcoord;

      // Is the texture flipped?
      float uv2xArea = t1.x * t2.y - t1.y * t2.x;
      if ( math::abs( uv2xArea ) < 0x1p-20 )
        continue; // Smaller than 1/2 pixel at 1024x1024
      float flip = uv2xArea > 0 ? 1 : -1;
      // 'flip' or '-flip'; depends on the handedness of the space.
      if ( verts[i].tangent.w != 0 && verts[i].tangent.w != -flip )
        inconsistents++;
      verts[i].tangent.w = -flip;

      // Project triangle onto tangent plane
      v1 -= n * math::dot( v1, n );
      v2 -= n * math::dot( v2, n );
      // Tangent is object space direction of texture coordinates
      vec3 s = math::normalize( ( t2.y * v1 - t1.y * v2 ) * flip );

      // Use angle between projected v1 and v2 as weight
      float angle = math::acos( math::dot( v1, v2 ) / ( math::length( v1 ) * math::length( v2 ) ) );
      verts[i].tangent += vec4( s * angle, 0 );
    }

    for ( GLuint l = 0; l < indices.size(); ++l )
    {
      vec4& t = verts[indices[l]].tangent;
      t = vec4( normalize( vec3( t.x, t.y, t.z ) ), t.w );
    }
  }*/

  inline vec2 tov2( const fbxsdk::FbxVector2& v2 )
  {
    return vec2( (float)v2.mData[0], (float)v2.mData[1] );
  };

  inline vec3 tov3( const fbxsdk::FbxVector4& v4 )
  {
    return vec3( (float)v4.mData[0], (float)v4.mData[1], (float)v4.mData[2] );
  };

  inline vec3 tov3( const FbxDouble3& v4 )
  {
    return vec3( (float)v4[0], (float)v4[1], (float)v4[2] );
  };

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

  void PrintAttribute( FbxNodeAttribute* attribute )
  {
    if ( !attribute || attribute->GetAttributeType() == FbxNodeAttribute::eNull )
      return;
    FbxString typeName = GetAttributeTypeName( attribute->GetAttributeType() );
    FbxString attrName = attribute->GetName();
    Locator::console().printf( Console::srcLoader, "<attribute type='%s' name='%s'/>", typeName.Buffer(), attrName.Buffer() );
  }

  struct SceneNode {
    vec3 translate_;
    quaternion rotate_;
    vec3 scale_;
  };

  void parseFBXNode( FbxNode* node, vector<ModelLoadOutput>& out_models )
  {
    const char* nodeName = node->GetName();

    SceneNode out;
    out.translate_ = tov3( node->LclTranslation.Get() );
    out.scale_ = tov3( node->LclScaling.Get() );
    FbxDouble3 rotation = node->LclRotation.Get();
    FbxDouble3 scaling = node->LclScaling.Get();

    Locator::console().printf( Console::srcLoader,
      "<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>",
      nodeName,
      out.translate_[0], out.translate_[1], out.translate_[2],
      rotation[0], rotation[1], rotation[2],
      out.scale_[0], out.scale_[1], out.scale_[2]
    );

    for ( int i = 0; i < node->GetNodeAttributeCount(); i++ )
    {
      auto attribute = node->GetNodeAttributeByIndex( i );
      PrintAttribute( attribute );
      if ( attribute->GetAttributeType() == FbxNodeAttribute::eMesh )
      {
        ModelLoadOutput model;
        auto mesh = static_cast<fbxsdk::FbxMesh*>( attribute );
        GLuint index_counter = 0;
        auto e_verts = mesh->GetControlPoints();
        auto polycount = mesh->GetPolygonCount();
        auto uvcount = mesh->GetElementUVCount();
        auto normalcount = mesh->GetElementNormalCount();
        auto tangentcount = mesh->GetElementTangentCount();
        Locator::console().printf( Console::srcLoader, "Mesh has %i polygons, %i UV elements, %i normals, %i tangents", polycount, uvcount, normalcount, tangentcount );
        assert( uvcount == 1 && normalcount == 1 );
        for ( int j = 0; j < polycount; j++ )
        {
          auto polysize = mesh->GetPolygonSize( j );
          assert( polysize == 3 || polysize == 4 );
          model.indices.push_back( index_counter + 0 );
          model.indices.push_back( index_counter + 1 );
          model.indices.push_back( index_counter + 2 );
          if ( polysize == 4 )
          {
            model.indices.push_back( index_counter + 2 );
            model.indices.push_back( index_counter + 3 );
            model.indices.push_back( index_counter + 0 );
          }
          index_counter += (GLuint)polysize;
          for ( int k = 0; k < polysize; k++ )
          {
            auto idx = mesh->GetPolygonVertex( j, k );
            Vertex3D out;
            out.position = tov3( e_verts[idx] );
            for ( int l = 0; l < uvcount; l++ )
            {
              auto e_uvs = mesh->GetElementUV( l );
              int index = -1;
              if ( e_uvs->GetMappingMode() == FbxGeometryElement::eByControlPoint )
              {
                if ( e_uvs->GetReferenceMode() == FbxGeometryElement::eDirect )
                  index = idx;
                else if ( e_uvs->GetReferenceMode() == FbxGeometryElement::eIndexToDirect )
                  index = e_uvs->GetIndexArray().GetAt( idx );
              } else if ( e_uvs->GetMappingMode() == FbxGeometryElement::eByPolygonVertex && e_uvs->GetReferenceMode() == FbxGeometryElement::eIndexToDirect )
                index = mesh->GetTextureUVIndex( j, k );
              assert( index >= 0 );
              out.texcoord = tov2( e_uvs->GetDirectArray().GetAt( index ) );
            }
            for ( int l = 0; l < normalcount; l++ )
            {
              auto e_normals = mesh->GetElementNormal( l );
              int index = -1;
              assert( e_normals->GetMappingMode() == FbxGeometryElement::eByPolygonVertex );
              if ( e_normals->GetReferenceMode() == FbxGeometryElement::eDirect )
                index = idx;
              else if ( e_normals->GetReferenceMode() == FbxGeometryElement::eIndexToDirect )
                index = e_normals->GetIndexArray().GetAt( idx );
              assert( index >= 0 );
              out.normal = tov3( e_normals->GetDirectArray().GetAt( index ) );
            }
            for ( int l = 0; l < tangentcount; l++ )
            {
              auto e_tangents = mesh->GetElementTangent( l );
              int index = -1;
              assert( e_tangents->GetMappingMode() == FbxGeometryElement::eByPolygonVertex );
              if ( e_tangents->GetReferenceMode() == FbxGeometryElement::eDirect )
                index = idx;
              else if ( e_tangents->GetReferenceMode() == FbxGeometryElement::eIndexToDirect )
                index = e_tangents->GetIndexArray().GetAt( idx );
              assert( index >= 0 );
              out.tangent = vec4( tov3( e_tangents->GetDirectArray().GetAt( index ) ), 1.0f );
            }
            model.vertices.push_back( move( out ) );
          }
        }
        util::generateTangentsAndBitangents( model.vertices, model.indices );
        /*for ( auto& vert : model.vertices )
        {
          Locator::console().printf( Console::srcEngine, "Vertex: pos %.2f %.2f %.2f normal %.2f %.2f %.2f uv %.2f %.2f tangent %.2f %.2f %.2f bitangent %.2f %.2f %.2f",
            vert.position.x, vert.position.y, vert.position.z,
            vert.normal.x, vert.normal.y, vert.normal.z,
            vert.texcoord.x, vert.texcoord.y,
            vert.tangent.x, vert.tangent.y, vert.tangent.z,
            vert.bitangent.x, vert.bitangent.y, vert.bitangent.z );
        }*/
        out_models.push_back( move( model ) );
      }
    }
    for ( int j = 0; j < node->GetChildCount(); j++ )
      parseFBXNode( node->GetChild( j ), out_models );
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
          assert( root );
          vector<ModelLoadOutput> out;
          parseFBXNode( root, out );
          finishedTasksLock_.lock();
          finishedModels_.insert( finishedModels_.end(), out.begin(), out.end() );
          finishedTasksLock_.unlock();
        }
        importer->Destroy( true );
      }
    }

    if ( !finishedMaterials_.empty() )
      finishedMaterialsEvent_.set();

    if ( !finishedFonts_.empty() )
      finishedFontsEvent_.set();

    if ( !finishedModels_.empty() )
      finishedModelsEvent_.set();
  }

  ThreadedLoader::~ThreadedLoader()
  {
    if ( fbxio_ )
      fbxio_->Destroy( true );
    if ( fbxmgr_ )
      fbxmgr_->Destroy();
  }

}