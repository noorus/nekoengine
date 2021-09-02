#include "stdafx.h"
#include "loader.h"
#include "utilities.h"
#include "lodepng.h"
#include "gfx_types.h"
#include "renderer.h"
#include "console.h"

#ifndef NEKO_NO_FBX

// Oops, can't do this after I statically linked the FBX libraries to OZZ
// #pragma comment( lib, "libfbxsdk.lib" )

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

  inline quaternion gltf_toQuat( const FbxDouble3& eul )
  {
    return math::quaternionFrom( (Real)eul[0], (Real)eul[1], (Real)eul[2] );
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

  static FbxAMatrix _parent_global_inv;

  inline vec3 safenormalize( vec3 in, vec3 safe )
  {
    const auto len2 = in.x * in.x + in.y * in.y + in.z * in.z;
    if ( len2 <= 0.0f )
      return safe;
    const auto len = math::sqrt( len2 );
    return ( in / len );
  }

  inline vec4 safenormalize( vec4 in, vec4 safe )
  {
    const auto len2 = in.x * in.x + in.y * in.y + in.z * in.z + in.w * in.w;
    if ( len2 <= 0.0f )
      return safe;
    const auto len = math::sqrt( len2 );
    return ( in / len );
  }

  void FbxLoader::parseFBXNode( fbxsdk::FbxNode* node, SceneNode& out )
  {
    const FbxAMatrix node_global = node->EvaluateGlobalTransform();
    const FbxAMatrix node_local = _parent_global_inv * node_global;

    ozz::math::Transform trans;
    if ( !converter_->ConvertTransform( node_local, &trans ) )
      NEKO_EXCEPT( "FBX transform conversion failed" );
    out.setTranslate( vec3( trans.translation.x, trans.translation.y, trans.translation.z ) );
    out.setScale( vec3( trans.scale.x, trans.scale.y, trans.scale.z ) );
    out.setRotate( quaternion( trans.rotation.x, trans.rotation.y, trans.rotation.z, trans.rotation.w ) );
    out.name_ = node->GetName();

    _parent_global_inv = node_global.Inverse();

    for ( int i = 0; i < node->GetNodeAttributeCount(); i++ )
    {
      auto attribute = node->GetNodeAttributeByIndex( i );
      if ( attribute->GetAttributeType() == FbxNodeAttribute::eMesh )
      {
        out.mesh_ = make_shared<ModelLoadOutput>();
        auto mesh = static_cast<fbxsdk::FbxMesh*>( attribute );
        mesh->GenerateTangentsDataForAllUVSets( false, false );

        /*FbxMatrix globalTransform = mesh->GetNode()->GetScene()->GetEvaluator()->GetNodeGlobalTransform( mesh->GetNode() );
        dvec4 c0 = glm::make_vec4( (double*)globalTransform.GetColumn( 0 ).Buffer() );
        dvec4 c1 = glm::make_vec4( (double*)globalTransform.GetColumn( 1 ).Buffer() );
        dvec4 c2 = glm::make_vec4( (double*)globalTransform.GetColumn( 2 ).Buffer() );
        dvec4 c3 = glm::make_vec4( (double*)globalTransform.GetColumn( 3 ).Buffer() );
        glm::mat4 convertMatr = mat4( c0, c1, c2, c3 );
        convertMatr = inverse( convertMatr ); //need to inverse otherwise the model is upside down*/

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
            auto ozzp = converter_->ConvertPoint( e_verts[idx] );
            vout.position = vec3( ozzp.x, ozzp.y, ozzp.z );
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
              assert( e_normals->GetReferenceMode() == FbxGeometryElement::eDirect );
              if ( e_normals->GetReferenceMode() == FbxGeometryElement::eDirect )
                index = idx;
              else if ( e_normals->GetReferenceMode() == FbxGeometryElement::eIndexToDirect )
                index = e_normals->GetIndexArray().GetAt( idx );
              assert( index >= 0 );
              auto ozzv = converter_->ConvertVector( e_normals->GetDirectArray().GetAt( index ) );
              vout.normal = safenormalize( vec3( ozzv.x, ozzv.y, ozzv.z ), vec3( 0.0f, 1.0f, 0.0f ) );
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
              auto ozzv = converter_->ConvertVector( e_tangents->GetDirectArray().GetAt( index ) );
              auto ov3 = safenormalize( vec3( ozzv.x, ozzv.y, ozzv.z ), vec3( 1.0f, 0.0f, 0.0f ) );
              vout.tangent = vec4( ov3.x, ov3.y, ov3.z, (float)e_tangents->GetDirectArray().GetAt( index )[3] );
            }
            out.mesh_->vertices_.push_back( move( vout ) );
          }
        }
        //util::generateTangentsAndBitangents( out.mesh_->vertices_, out.mesh_->indices_ );
        for ( auto& vert : out.mesh_->vertices_ )
        {
          Locator::console().printf( Console::srcEngine, "Vertex: pos %.2f %.2f %.2f normal %.2f %.2f %.2f uv %.2f %.2f tangent %.2f %.2f %.2f bitangent %.2f %.2f %.2f",
            vert.position.x, vert.position.y, vert.position.z,
            vert.normal.x, vert.normal.y, vert.normal.z,
            vert.texcoord.x, vert.texcoord.y,
            vert.tangent.x, vert.tangent.y, vert.tangent.z,
            vert.bitangent.x, vert.bitangent.y, vert.bitangent.z );
        }
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
        size = ( static_cast<int>( data_.size_bytes() ) - pos_ );
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
      if ( !importer->Import( scene ) )
        NEKO_EXCEPT( "FBX import failed" );

      auto& settings = scene->GetGlobalSettings();
      converter_ = make_unique<ozz::animation::offline::fbx::FbxSystemConverter>( settings.GetAxisSystem(), settings.GetSystemUnit() );

      /*fbxsdk::FbxAxisSystem axis( fbxsdk::FbxAxisSystem::eOpenGL );
      axis.DeepConvertScene( scene );
      fbxsdk::FbxSystemUnit::ConversionOptions opts = { true, true, true, true, true, true };
      fbxsdk::FbxSystemUnit::m.ConvertScene( scene, opts );*/

      auto root = scene->GetRootNode();
      assert( root );
      _parent_global_inv = FbxAMatrix();
      parseFBXNode( root, *rootNode );
      rootNode->setTranslate( vec3( 0.0f, 2.0f, 0.0f ) );
      // rootNode->setRotate( math::angleAxis( math::radians( -90.0f ), vec3UnitX ) );
    }
    importer->Destroy( true );
  }

}

#endif