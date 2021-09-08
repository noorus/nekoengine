#include "stdafx.h"
#ifndef NEKO_NO_SCRIPTING

#include "js_util.h"
#include "console.h"
#include "scripting.h"
#include "nekomath.h"
#include "js_math.h"
#include "js_mathutil.h"
#include "js_mesh.h"
#include "locator.h"

namespace neko {

  JSMesh::JSMesh()
  {
    static size_t indexCounter = 0;
    id_ = indexCounter++;
  }

  namespace js {

    static const char* c_className = "mesh";

    string Mesh::className( c_className );
    WrappedType Mesh::internalType = Wrapped_Mesh;

    void Mesh::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      //JS_WRAPPER_SETACCESSOR( tpl, Mesh, x, X );
      //JS_WRAPPER_SETACCESSOR( tpl, Mesh, y, Y );

      // Methods
      JS_WRAPPER_SETMEMBER( tpl, Mesh, toString );
    }

    inline void extractVBO_2D( v8::Array& arr, V8Context& context, VBOPtr& vbo )
    {
      vector<Vertex2D> verts;
      verts.resize( arr.Length() );
      for ( uint32_t i = 0; i < arr.Length(); ++i )
      {
        auto arrayValue = v8::Local<v8::Array>::Cast( arr.Get( context, i ).ToLocalChecked() );
        verts[i].x = static_cast<Real>( arrayValue->Get( context, 0 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].y = static_cast<Real>( arrayValue->Get( context, 1 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].s = static_cast<Real>( arrayValue->Get( context, 2 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].t = static_cast<Real>( arrayValue->Get( context, 3 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
      }
      vbo->pushVertices( move( verts ) );
    }

    inline void extractVBO( v8::Array& arr, V8Context& context, VBOPtr& vbo )
    {
      vector<Vertex3D> verts;
      verts.resize( arr.Length() );
      for ( uint32_t i = 0; i < arr.Length(); ++i )
      {
        auto arrayValue = v8::Local<v8::Array>::Cast( arr.Get( context, i ).ToLocalChecked() );
        verts[i].position.x = static_cast<Real>( arrayValue->Get( context, 0 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].position.y = static_cast<Real>( arrayValue->Get( context, 1 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].position.z = static_cast<Real>( arrayValue->Get( context, 2 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].normal = vec3( 0.0f );
        verts[i].texcoord.x = static_cast<Real>( arrayValue->Get( context, 3 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].texcoord.y = static_cast<Real>( arrayValue->Get( context, 4 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].color = vec4( 1.0f, 1.0f, 1.0f, 1.0f );
      }
      vbo->pushVertices( move( verts ) );
    }

    inline void extractEBO( v8::Array& arr, V8Context& context, EBOPtr& ebo )
    {
      vector<GLuint> indices;
      indices.reserve( arr.Length() );
      for ( uint32_t i = 0; i < arr.Length(); ++i )
      {
        auto value = arr.Get( context, i ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 );
        indices.push_back( static_cast<GLuint>( value ) );
      }
      ebo->storage_.insert( ebo->storage_.end(), indices.begin(), indices.end() );
      ebo->dirty_ = true;
    }

    void Mesh::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      auto context = args.GetIsolate()->GetCurrentContext();

      JSMesh mesh;
      mesh.vbo_ = make_shared<VBO>( VBOType::VBO_3D, false );
      mesh.ebo_ = make_shared<EBO>();
      mesh.vao_.reset();

      bool valueParsed = false;
      if ( args.Length() == 2 && args[0]->IsArray() && args[1]->IsArray() )
      {
        auto vboArray = v8::Local<v8::Array>::Cast( args[0] );
        extractVBO( **vboArray, context, mesh.vbo_ );

        auto eboArray = v8::Local<v8::Array>::Cast( args[1] );
        extractEBO( **eboArray, context, mesh.ebo_ );

        valueParsed = true;
      }
      else if ( args.Length() > 1 && args[0]->IsString() )
      {
        v8::String::Utf8Value name( isolate, args[0] );
        if ( *name )
        {
          // MeshGenerator: Plane
          if ( _stricmp( *name, "plane" ) == 0 )
          {
            if ( args.Length() != 4 )
            {
              isolate->ThrowException( util::staticStr( isolate, "Mesh constructor 'plane' requires arguments: vec2 dimensions, vec2 segments, vec3 normal" ) );
              return;
            }
            auto dimensions = extractVector2( 1, args );
            auto segments = extractVector2( 2, args );
            auto normal = extractVector3( 3, args );
            if ( !dimensions || !segments || !normal )
            {
              isolate->ThrowException( util::staticStr( isolate, "Mesh constructor 'plane' requires arguments: vec2 dimensions, vec2 segments, vec3 normal" ) );
              return;
            }
            vec2u segmentsRounded = glm::round( segments->v() );
            if ( segmentsRounded.x > 0 && segmentsRounded.y > 0 )
            {
              auto retPair = Locator::meshGenerator().makePlane( dimensions->v(), segmentsRounded, normal->v() );
              mesh.vbo_->pushVertices( move( retPair.first ) );
              mesh.ebo_->storage_.insert( mesh.ebo_->storage_.end(), retPair.second.begin(), retPair.second.end() );
              mesh.ebo_->dirty_ = true;
              valueParsed = true;
            }
          }
          else if ( _stricmp( *name, "ground" ) == 0 )
          {
            if ( args.Length() != 3 )
            {
              isolate->ThrowException( util::staticStr( isolate, "Mesh constructor 'ground' requires arguments: vec3 dimensions, vec2 segments" ) );
              return;
            }
            auto dimensions = extractVector3( 1, args );
            auto segments = extractVector2( 2, args );
            if ( !dimensions || !segments )
            {
              isolate->ThrowException( util::staticStr( isolate, "Mesh constructor 'ground' requires arguments: vec3 dimensions, vec2 segments" ) );
              return;
            }
            vec2i segmentsRounded = glm::round( segments->v() );
            if ( segmentsRounded.x > 0 && segmentsRounded.y > 0 )
            {
              auto retPair = Locator::meshGenerator().makeTerrain( segmentsRounded, dimensions->v() );
              mesh.vbo_->pushVertices( move( retPair.first ) );
              mesh.ebo_->storage_.insert( mesh.ebo_->storage_.end(), retPair.second.begin(), retPair.second.end() );
              mesh.ebo_->dirty_ = true;
              valueParsed = true;
            }
          }
        }
      }

      if ( !valueParsed )
      {
        isolate->ThrowException( util::staticStr( isolate, "Invalid constructor call to mesh" ) );
        return;
      }

      auto ctx = getScriptContext( isolate );
      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr = ctx->meshreg().createFromJS( thisObj, mesh );
        ctx->renderSync().constructed( ptr.get() );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = ctx->meshreg().createFrom( mesh );
        ctx->renderSync().constructed( ptr.get() );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    void Mesh::js_toString( const V8CallbackArgs& args )
    {
      char result[48];
      sprintf_s<48>( result, "mesh[%i(%zi),%i(%zi),%i]",
        local_.vbo_->id_, local_.vbo_->vertexCount(),
        local_.ebo_->id_, local_.ebo_->storage_.size(),
        ( local_.vao_ ? local_.vao_->id_ : 0 ) );
      args.GetReturnValue().Set( util::allocString( result, args.GetIsolate() ) );
    }

    int32_t Mesh::jsEstimateSize() const
    {
      int64_t result = sizeof( Mesh );
      if ( local_.ebo_ )
        result += local_.ebo_->sizeInBytes();
      if ( local_.vbo_ )
        result += local_.vbo_->sizeInBytes();
      if ( local_.vao_ )
        result += sizeof( VAO );
      return static_cast<int32_t>( std::min( static_cast<int64_t>( std::numeric_limits<int32_t>::max() ), result ) );
    }

    void Mesh::jsOnDestruct( Isolate* isolate )
    {
      if ( !isolate )
        return;
      auto ctx = getScriptContext( isolate );
      if ( ctx )
        ctx->renderSync().destructed( this );
    }

    MeshPtr extractMeshMember( Isolate* isolate, const utf8String& func, v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name, bool shouldThrow )
    {
      const MeshPtr emptyPtr;
      if ( maybeObject.IsEmpty() )
      {
        if ( shouldThrow )
          util::throwException( isolate, ( "Syntax error: " + func + ": passed object is empty" ).c_str() );
        return emptyPtr;
      }
      auto object = maybeObject.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
      if ( object.IsEmpty() )
      {
        if ( shouldThrow )
          util::throwException( isolate, ( func + ": passed object has no mesh member \"" + name + "\"" ).c_str() );
        return emptyPtr;
      }
      auto local = v8::Local<v8::Object>::Cast( object.ToLocalChecked() );
      if ( !util::isWrappedType( isolate->GetCurrentContext(), local, Wrapped_Mesh ) )
      {
        if ( shouldThrow )
          util::throwException( isolate, ( func + ": passed object's member is not a mesh \"" + name + "\"" ).c_str() );
        return emptyPtr;
      }
      return move( Mesh::unwrap( local )->shared_from_this() );
    }

  }

}

#endif