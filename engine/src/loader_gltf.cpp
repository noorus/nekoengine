#include "pch.h"
#include "loader.h"
#include "utilities.h"
#include "lodepng.h"
#include "gfx_types.h"
#include "renderer.h"
#include "console.h"
#include "tiny_gltf.h"

namespace neko::loaders {

  static size_t ComponentTypeByteSize( int type )
  {
    switch ( type )
    {
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      case TINYGLTF_COMPONENT_TYPE_BYTE:
        return sizeof( char );
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      case TINYGLTF_COMPONENT_TYPE_SHORT:
        return sizeof( short );
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      case TINYGLTF_COMPONENT_TYPE_INT:
        return sizeof( int );
      case TINYGLTF_COMPONENT_TYPE_FLOAT:
        return sizeof( float );
      case TINYGLTF_COMPONENT_TYPE_DOUBLE:
        return sizeof( double );
      default:
        return 0;
    }
  }

  struct DumbBufferStructure {
    vector<uint8_t> data_;
    utf8String viewName_;
    utf8String bufferName_;
    GLenum componentType_ = GLenum::GL_FAILURE_NV; // as in what's fed to glVertexAttribPointer as [type]; GL_BYTE, GL_INT and so on
  };

  static void gltf_extractBuffers( tinygltf::Model& model, vector<DumbBufferStructure>& out )
  {
    for ( size_t i = 0; i < model.bufferViews.size(); i++ )
    {
      DumbBufferStructure out_buf;

      const tinygltf::BufferView& bufferView = model.bufferViews[i];
      if ( bufferView.target == 0 )
        NEKO_EXCEPT( "GLTF buffer view target is zero" );

      int sparse_accessor = -1;
      for ( size_t a_i = 0; a_i < model.accessors.size(); ++a_i )
      {
        const auto& accessor = model.accessors[a_i];
        if ( accessor.bufferView == i )
        {
          if ( out_buf.componentType_ == GLenum::GL_FAILURE_NV )
            out_buf.componentType_ = (GLenum)accessor.componentType;
          else if ( out_buf.componentType_ != (GLenum)accessor.componentType )
            NEKO_EXCEPT( "GLTF accessors for bufferview have multiple component types" );
          if ( accessor.sparse.isSparse )
          {
            // "WARN: this bufferView has at least one sparse accessor to it. We are going to load the data as patched by this sparse accessor, not the original data"
            sparse_accessor = (int)a_i;
            break;
          }
        }
      }

      const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

      out_buf.data_.resize( bufferView.byteLength );

      if ( sparse_accessor < 0 )
      {
        memcpy( out_buf.data_.data(), &buffer.data.at( 0 ) + bufferView.byteOffset, bufferView.byteLength );
      }
      else
      {
        const auto& accessor = model.accessors[sparse_accessor];

        memcpy( out_buf.data_.data(), buffer.data.data() + bufferView.byteOffset, bufferView.byteLength );

        const size_t size_of_object_in_buffer = ComponentTypeByteSize( accessor.componentType );
        const size_t size_of_sparse_indices = ComponentTypeByteSize( accessor.sparse.indices.componentType );

        const auto& indices_buffer_view = model.bufferViews[accessor.sparse.indices.bufferView];
        const auto& indices_buffer = model.buffers[indices_buffer_view.buffer];

        const auto& values_buffer_view = model.bufferViews[accessor.sparse.values.bufferView];
        const auto& values_buffer = model.buffers[values_buffer_view.buffer];

        for ( size_t sparse_index = 0; sparse_index < accessor.sparse.count; ++sparse_index )
        {
          int index = 0;
          // std::cout << "accessor.sparse.indices.componentType = " <<
          // accessor.sparse.indices.componentType << std::endl;
          switch ( accessor.sparse.indices.componentType )
          {
            case TINYGLTF_COMPONENT_TYPE_BYTE:
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
              index = (int)*(unsigned char*)( indices_buffer.data.data() + indices_buffer_view.byteOffset + accessor.sparse.indices.byteOffset + ( sparse_index * size_of_sparse_indices ) );
              break;
            case TINYGLTF_COMPONENT_TYPE_SHORT:
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
              index = (int)*(unsigned short*)( indices_buffer.data.data() + indices_buffer_view.byteOffset + accessor.sparse.indices.byteOffset + ( sparse_index * size_of_sparse_indices ) );
              break;
            case TINYGLTF_COMPONENT_TYPE_INT:
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
              index = (int)*(unsigned int*)( indices_buffer.data.data() + indices_buffer_view.byteOffset + accessor.sparse.indices.byteOffset + ( sparse_index * size_of_sparse_indices ) );
              break;
          }

          const unsigned char* read_from = values_buffer.data.data() + ( values_buffer_view.byteOffset + accessor.sparse.values.byteOffset ) + ( sparse_index * ( size_of_object_in_buffer * accessor.type ) );

          unsigned char* write_to = out_buf.data_.data() + index * ( size_of_object_in_buffer * accessor.type );

          memcpy( write_to, read_from, size_of_object_in_buffer * accessor.type );
        }
      }

      out.push_back( move( out_buf ) );
    }
  };

  constexpr vec3 gltf_toVec3( const vector<double>& v, const vec3 def = vec3( 0.0f ) )
  {
    if ( v.empty() )
      return def;
    return { static_cast<Real>( v[0] ), static_cast<Real>( v[1] ), static_cast<Real>( v[2] ) };
  }

  constexpr quaternion gltf_toQuat( const vector<double>& v, const quaternion def = glm::quat_identity<Real, glm::defaultp>() )
  {
    if ( v.empty() )
      return def;
    return { static_cast<Real>( v[0] ), static_cast<Real>( v[1] ), static_cast<Real>( v[2] ), static_cast<Real>( v[3] ) };
  }

  constexpr vec2 gltf_mesh_vec2( vec2 a )
  {
    return { a.x, a.y };
  }

  constexpr vec3 gltf_mesh_vec3( vec3 a )
  {
    return { a.x, a.y, a.z };
  }

  constexpr vec4 gltf_mesh_vec4( vec4 a )
  {
    return { a.x, a.y, a.z, a.w };
  }

  void gltf_traverseTree( MeshNodePtr out, const tinygltf::Node& node, const vector<DumbBufferStructure>& buffers,
    const tinygltf::Model& model, int depth = 0 )
  {
    out->name = node.name;

    out->scale = gltf_toVec3( node.scale, vec3( 1.0f ) );
    out->rotate = gltf_toQuat( node.rotation );
    out->translate = gltf_toVec3( node.translation, vec3( 0.0f ) );

    Locator::console().printf( srcLoader, "gltf: node %s depth %i trans %f,%f,%f scl %f,%f,%f mesh %i",
      node.name.c_str(), depth, out->translate.x, out->translate.x, out->translate.y, out->scale.x, out->scale.y,
      out->scale.z, node.mesh );

    if ( node.mesh >= 0 )
    {
      auto& mesh = model.meshes[node.mesh];
      for ( auto& primitive : mesh.primitives )
      {
        if ( primitive.indices < 0 )
          continue;
        const auto& indices = model.accessors[primitive.indices];
        Locator::console().printf( srcLoader, "gltf: mesh primitive indices %i count %i mode %i", primitive.indices, indices.count, primitive.mode );
        int pos_acc = -1;
        int pos_count = -1;
        int norm_acc = -1;
        int tang_acc = -1;
        int tc_acc = -1;
        for ( auto& attribute : primitive.attributes )
        {
          const auto& accessor = model.accessors[attribute.second];
          if ( attribute.first == "POSITION" )
          {
            if ( accessor.type != TINYGLTF_TYPE_VEC3 )
              NEKO_EXCEPT( "GLTF position accessor is not a vec3" );
            pos_acc = accessor.bufferView;
            pos_count = (int)accessor.count;
          }
          else if ( attribute.first == "NORMAL" )
          {
            if ( accessor.type != TINYGLTF_TYPE_VEC3 )
              NEKO_EXCEPT( "GLTF normal accessor is not a vec3" );
            norm_acc = accessor.bufferView;
          }
          else if ( attribute.first == "TANGENT" )
          {
            if ( accessor.type != TINYGLTF_TYPE_VEC4 )
              NEKO_EXCEPT( "GLTF tangent accessor is not a vec4" );
            tang_acc = accessor.bufferView;
          }
          else if ( attribute.first == "TEXCOORD_0" )
          {
            if ( accessor.type != TINYGLTF_TYPE_VEC2 )
              NEKO_EXCEPT( "GLTF texcoord accessor is not a vec2" );
            tc_acc = accessor.bufferView;
          }
          Locator::console().printf( srcLoader, "gltf: mesh primitive attribute %s buffer %i name %s viewname %s componenttype %i size %i", attribute.first.c_str(), accessor.bufferView, buffers[accessor.bufferView].bufferName_.c_str(), buffers[accessor.bufferView].viewName_.c_str(), buffers[accessor.bufferView].componentType_, buffers[accessor.bufferView].data_.size() );
        }
        if ( pos_acc < 0 || norm_acc < 0 || tang_acc < 0 || tc_acc < 0 )
          NEKO_EXCEPT( "GLTF mesh is missing pos, norm, tang or texcoord array" );
        auto index_data = buffers[indices.bufferView].data_.data();
        auto pos_data = reinterpret_cast<const vec3*>( buffers[pos_acc].data_.data() );
        auto norm_data = reinterpret_cast<const vec3*>( buffers[norm_acc].data_.data() );
        auto tang_data = reinterpret_cast<const vec4*>( buffers[tang_acc].data_.data() );
        auto tc_data = reinterpret_cast<const vec2*>( buffers[tc_acc].data_.data() );
        for ( int i = 0; i < pos_count; i++ )
        {
          Vertex3D v;
          v.position = gltf_mesh_vec3( *( pos_data + i ) );
          v.normal = gltf_mesh_vec3( *( norm_data + i ) );
          v.tangent = gltf_mesh_vec4( *( tang_data + i ) );
          v.texcoord = gltf_mesh_vec2( * ( tc_data + i ) );
          v.color = vec4( 1.0f, 1.0f, 1.0f, 1.0f );
          v.bitangent = vec3( 0.0f );
          out->vertices.push_back( v );
        }
        out->indices.resize( indices.count );
        for ( int i = 0; i < indices.count; i++ )
        {
          switch ( indices.componentType )
          {
            case TINYGLTF_COMPONENT_TYPE_BYTE:
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
              out->indices[i] = static_cast<GLuint>( *( (unsigned char*)( index_data ) + i ) );
              break;
            case TINYGLTF_COMPONENT_TYPE_SHORT:
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
              out->indices[i] = static_cast<GLuint>( *( (unsigned short*)( index_data ) + i ) );
              break;
            case TINYGLTF_COMPONENT_TYPE_INT:
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
              out->indices[i] = static_cast<GLuint>( *( (unsigned int*)( index_data ) + i ) );
              break;
          }
        }
      }
      //mesh.primitives[0].
    }
    for ( auto childindex : node.children )
    {
      auto child = make_shared<MeshNode>();
      auto& childnode = model.nodes[childindex];
      gltf_traverseTree( child, childnode, buffers, model, depth + 1 );
      out->children.insert( child );
    }
  }

  void loadGLTFModel( const vector<uint8_t>& input, const utf8String& filename, const utf8String& basedir, MeshNodePtr out )
  {
    tinygltf::TinyGLTF context;
    tinygltf::Model model;
    utf8String error;
    utf8String warning;
    auto buf = reinterpret_cast<char*>( Locator::memory().allocZeroed( Memory::Sector::Graphics, input.size() + 1 ) );
    memcpy( buf, input.data(), input.size() + 1 );
    if ( !context.LoadASCIIFromString( &model, &error, &warning, buf, static_cast<unsigned int>( input.size() ), basedir ) )
      NEKO_EXCEPT( "GLTF load failed\n" + error );
    auto nodecount = model.nodes.size();
    auto meshcount = model.meshes.size();
    auto matcount = model.materials.size();
    auto skincount = model.skins.size();
    Locator::console().printf( srcLoader,
      "gltf: tinygltf loaded %s, %i nodes %i meshes %i materials %i skins",
      filename.c_str(), nodecount, meshcount, matcount, skincount );
    if ( !model.nodes.empty() )
    {
      vector<DumbBufferStructure> buffers;
      gltf_extractBuffers( model, buffers );
      gltf_traverseTree( out, model.nodes[0], buffers, model );
    }
  }

}