#include "stdafx.h"
#include "loader.h"
#include "utilities.h"
#include "lodepng.h"
#include "gfx_types.h"
#include "renderer.h"
#include "console.h"

namespace neko::loaders {

  inline vec2 tov2( const fbxsdk::FbxVector2& v2 )
  {
    return vec2( (float)v2.mData[0], (float)v2.mData[1] );
  }

  inline vec3 tov3( const fbxsdk::FbxVector4& v4 )
  {
    return vec3( (float)v4.mData[0], (float)v4.mData[1], (float)v4.mData[2] );
  }

  inline vec3 tov3( const FbxDouble3& v4 )
  {
    return vec3( (float)v4[0], (float)v4[1], (float)v4[2] );
  }

  inline quaternion toq( const FbxDouble3& eul )
  {
    return math::quaternionFrom( (Real)eul[0], (Real)eul[1], (Real)eul[2] );
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

  void dumpSceneGraph( SceneNode& root, int level = 0 )
  {
    utf8String prep;
    for ( int i = 0; i < level; i++ )
      prep.append( "  " );
    Locator::console().printf( Console::srcGame, "%s<node \"%s\" pos %.2f %.2f %.2f, scale %.2f %.2f %.2f, rot %.2f %.2f %.2f %.2f>",
      prep.c_str(), root.name_.c_str(),
      root.translate_.x, root.translate_.y, root.translate_.z,
      root.scale_.x, root.scale_.y, root.scale_.z,
      root.rotate_.x, root.rotate_.y, root.rotate_.z, root.rotate_.z
    );
    for ( auto& child : root.children_ )
      dumpSceneGraph( *child, level + 1 );
  }

  FbxLoader::FbxLoader(): fbxio_( nullptr ), fbxmgr_( nullptr )
  {
    fbxmgr_ = FbxManager::Create();
    fbxio_ = FbxIOSettings::Create( fbxmgr_, IOSROOT );
    fbxmgr_->SetIOSettings( fbxio_ );
  }

  FbxLoader::~FbxLoader()
  {
    if ( fbxio_ )
      fbxio_->Destroy( true );
    if ( fbxmgr_ )
      fbxmgr_->Destroy();
  }

  void FbxLoader::parseFBXNode( fbxsdk::FbxNode* node, SceneNode& out )
  {
    out.setTranslate( tov3( node->LclTranslation.Get() ) );
    out.setScale( tov3( node->LclScaling.Get() ) );
    out.setRotate( toq( node->LclRotation.Get() ) );
    out.name_ = node->GetName();

    for ( int i = 0; i < node->GetNodeAttributeCount(); i++ )
    {
      auto attribute = node->GetNodeAttributeByIndex( i );
      if ( attribute->GetAttributeType() == FbxNodeAttribute::eMesh )
      {
        out.mesh_ = make_shared<ModelLoadOutput>();
        auto mesh = static_cast<fbxsdk::FbxMesh*>( attribute );
        mesh->GenerateTangentsDataForAllUVSets( false, false );
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
          out.mesh_->indices_.push_back( index_counter + 0 );
          out.mesh_->indices_.push_back( index_counter + 1 );
          out.mesh_->indices_.push_back( index_counter + 2 );
          if ( polysize == 4 )
          {
            out.mesh_->indices_.push_back( index_counter + 2 );
            out.mesh_->indices_.push_back( index_counter + 3 );
            out.mesh_->indices_.push_back( index_counter + 0 );
          }
          index_counter += (GLuint)polysize;
          for ( int k = 0; k < polysize; k++ )
          {
            auto idx = mesh->GetPolygonVertex( j, k );
            Vertex3D vout;
            vout.position = tov3( e_verts[idx] );
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
              vout.texcoord = tov2( e_uvs->GetDirectArray().GetAt( index ) );
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
              vout.normal = tov3( e_normals->GetDirectArray().GetAt( index ) );
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
              vout.tangent = vec4( tov3( e_tangents->GetDirectArray().GetAt( index ) ), 0.0f );
            }
            out.mesh_->vertices_.push_back( move( vout ) );
          }
        }
        //util::generateTangentsAndBitangents( out.mesh_->vertices_, out.mesh_->indices_ );
        /*for ( auto& vert : out.mesh_->vertices_ )
        {
          Locator::console().printf( Console::srcEngine, "Vertex: pos %.2f %.2f %.2f normal %.2f %.2f %.2f uv %.2f %.2f tangent %.2f %.2f %.2f bitangent %.2f %.2f %.2f",
            vert.position.x, vert.position.y, vert.position.z,
            vert.normal.x, vert.normal.y, vert.normal.z,
            vert.texcoord.x, vert.texcoord.y,
            vert.tangent.x, vert.tangent.y, vert.tangent.z,
            vert.bitangent.x, vert.bitangent.y, vert.bitangent.z );
        }*/
      }
    }

    for ( int j = 0; j < node->GetChildCount(); j++ )
    {
      auto child = new SceneNode( &out );
      parseFBXNode( node->GetChild( j ), *child );
      out.children_.push_back( child );
    }
  }

  class FbxStreamAdapter: public fbxsdk::FbxStream {
  protected:
    mutable long pos_ = 0;
    mutable int err_ = 0;
    span<const uint8_t> data_;
  public:
    FbxStreamAdapter( span<const uint8_t> data ): data_( data )
    {
    };
    virtual ~FbxStreamAdapter() {};
    EState GetState() override
    {
      return eOpen;
    }
    bool Open( void* stream ) override
    {
      pos_ = 0;
      err_ = 0;
      return true;
    }
    bool Close() override
    {
      pos_ = 0;
      err_ = 0;
      return true;
    }
    bool Flush() override
    {
      err_ = 0;
      return true;
    }
    int Write( const void* data, int size ) override
    {
      err_ = 1;
      return 0;
    }
    int Read( void* data, int size ) const override
    {
      if ( size > ( data_.size_bytes() - pos_ ) )
        size = ( data_.size_bytes() - pos_ );
      memcpy( data, &data_.data()[pos_], size );
      pos_ = pos_ + size;
      return size;
    }
    int GetReaderID() const override
    {
      return 1337;
    }
    int GetWriterID() const override
    {
      return 1337;
    }
    void Seek( const fbxsdk::FbxInt64& offset, const fbxsdk::FbxFile::ESeekPos& seek ) override
    {
      if ( seek == fbxsdk::FbxFile::ESeekPos::eBegin )
        pos_ = static_cast<long>( offset );
      if ( seek == fbxsdk::FbxFile::ESeekPos::eCurrent )
        pos_ = pos_ + static_cast<long>( offset );
      if ( seek == fbxsdk::FbxFile::ESeekPos::eEnd )
        pos_ = static_cast<long>( data_.size_bytes() ) - static_cast<long>( offset );
      err_ = 0;
    }
    long GetPosition() const override
    {
      err_ = 0;
      return pos_;
    }
    void SetPosition( long pos ) override
    {
      err_ = 0;
      pos_ = pos;
    }
    int GetError() const override
    {
      return err_;
    }
    void ClearError() override
    {
      err_ = 0;
    }
  };

  void FbxLoader::loadFBXScene( const vector<uint8_t>& data, const utf8String& filename, SceneNode* rootNode )
  {
    auto importer = FbxImporter::Create( fbxmgr_, "" );
    FbxStreamAdapter stream( span( data.data(), data.size() ) );
    // if ( importer->Initialize( &stream, nullptr, -1, fbxmgr_->GetIOSettings() ) )
    if ( importer->Initialize( filename.c_str(), -1, fbxmgr_->GetIOSettings() ) )
    {
      auto scene = FbxScene::Create( fbxmgr_, "loaderScene" );
      importer->Import( scene );
      fbxsdk::FbxAxisSystem axis( fbxsdk::FbxAxisSystem::eOpenGL );
      axis.DeepConvertScene( scene );
      fbxsdk::FbxSystemUnit::ConversionOptions opts = { true, true, true, true, true, true };
      fbxsdk::FbxSystemUnit::m.ConvertScene( scene, opts );
      auto root = scene->GetRootNode();
      assert( root );
      parseFBXNode( root, *rootNode );
      rootNode->setRotate( math::angleAxis( math::radians( -90.0f ), vec3UnitX ) );
      dumpSceneGraph( *rootNode );
    }
    importer->Destroy( true );
  }

}