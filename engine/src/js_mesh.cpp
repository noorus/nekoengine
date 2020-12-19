#include "stdafx.h"
#include "js_util.h"
#include "console.h"
#include "scripting.h"
#include "nekomath.h"
#include "js_math.h"
#include "js_mathutil.h"
#include "js_mesh.h"

namespace neko {

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

    inline void extractVBO( v8::Array& arr, V8Context& context, VBOPtr& vbo )
    {
      vector<Vertex2D> verts;
      verts.resize( arr.Length() );
      for ( uint32_t i = 0; i < arr.Length(); ++i )
      {
        auto arrayValue = v8::Local<v8::Array>::Cast( arr.Get( i ) );
        verts[i].x = static_cast<Real>( arrayValue->Get( 0 )->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].y = static_cast<Real>( arrayValue->Get( 1 )->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].s = static_cast<Real>( arrayValue->Get( 2 )->NumberValue( context ).FromMaybe( 0.0 ) );
        verts[i].t = static_cast<Real>( arrayValue->Get( 3 )->NumberValue( context ).FromMaybe( 0.0 ) );
      }
      vbo->pushVertices( move( verts ) );
    }

    inline void extractEBO( v8::Array& arr, V8Context& context, EBOPtr& ebo )
    {
      vector<GLuint> indices;
      indices.reserve( arr.Length() );
      for ( uint32_t i = 0; i < arr.Length(); ++i )
      {
        auto value = arr.Get( i )->NumberValue( context ).FromMaybe( 0.0 );
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
      mesh.vbo_ = make_shared<VBO>( VBOType::VBO_2D );
      mesh.ebo_ = make_shared<EBO>();
      mesh.vao_.reset();

      bool valueParsed = false;
      if ( args.Length() == 2 )
      {
        if ( args[0]->IsArray() && args[1]->IsArray() )
        {
          auto vboArray = v8::Local<v8::Array>::Cast( args[0] );
          extractVBO( **vboArray, context, mesh.vbo_ );

          auto eboArray = v8::Local<v8::Array>::Cast( args[1] );
          extractEBO( **eboArray, context, mesh.ebo_ );

          valueParsed = true;
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
        ctx->renderSync().constructed( ptr );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = ctx->meshreg().createFrom( mesh );
        ctx->renderSync().constructed( ptr );
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

  }

}


