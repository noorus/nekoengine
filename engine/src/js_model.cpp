#include "stdafx.h"
#include "js_util.h"
#include "console.h"
#include "scripting.h"
#include "nekomath.h"
#include "js_math.h"
#include "js_mathutil.h"
#include "js_mesh.h"
#include "js_model.h"
#include "js_wrapper.h"

namespace neko {

  namespace js {

    static const char* c_className = "model";

    string Model::className( c_className );
    WrappedType Model::internalType = Wrapped_Model;

    JSModel::JSModel()
    {
      static size_t indexCounter = 0;
      id_ = indexCounter++;
    }

    void Model::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_WRAPPER_SETACCESSOR( tpl, Model, scale, Scale );
      JS_WRAPPER_SETACCESSOR( tpl, Model, translate, Translate );

      // Methods
      JS_WRAPPER_SETMEMBER( tpl, Model, toString );
    }

    void Model::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      auto context = args.GetIsolate()->GetCurrentContext();

      const utf8String funcName( "model::Constructor" );

      vec3 defaultScale( 1.0f, 1.0f, 1.0f );
      vec3 defaultTranslate( 0.0f, 0.0f, 0.0f );

      JSModel model;

      WrappedType wrappedType = Max_WrappedType;
      if ( args.Length() > 0 )
      {
        util::getWrappedType( context, args[0], wrappedType );
        if ( args[0]->IsObject() && wrappedType == Max_WrappedType )
        {
          auto maybeObj = args[0]->ToObject( context );
          model.mesh_ = extractMeshMember( isolate, funcName, maybeObj, "mesh", false );
          model.scale_ = extractVector3Member( isolate, funcName, maybeObj, "scale", false );
          model.translate_ = extractVector3Member( isolate, funcName, maybeObj, "translate", false );
        }
      }

      if ( !model.mesh_ )
      {
        isolate->ThrowException( util::staticStr( isolate, "No mesh in constructor call to model" ) );
        return;
      }

      auto ctx = getScriptContext( isolate );

      if ( !model.scale_ )
        model.scale_ = ctx->vec3reg().createFrom( defaultScale );
      if ( !model.translate_ )
        model.translate_ = ctx->vec3reg().createFrom( defaultTranslate );

      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr = ctx->modelreg().createFromJS( thisObj, model );
        ctx->renderSync().constructed( ptr );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = ctx->modelreg().createFrom( model );
        ctx->renderSync().constructed( ptr );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    void Model::js_getScale( V8String prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( local_.scale_->handle( info.GetIsolate() ) );
    }

    void Model::js_setScale( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto isolate = info.GetIsolate();
      auto context = isolate->GetCurrentContext();

      WrappedType argWrapType = Max_WrappedType;
      if ( !util::getWrappedType( context, value, argWrapType ) || argWrapType != Wrapped_Vector3 )
      {
        isolate->ThrowException( util::staticStr( isolate, "Passed argument is not a vec3" ) );
        return;
      }

      auto object = value->ToObject( context ).ToLocalChecked();
      local_.scale_ = Vector3::unwrap( object )->shared_from_this();
    }

    void Model::js_getTranslate( V8String prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( local_.translate_->handle( info.GetIsolate() ) );
    }

    void Model::js_setTranslate( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto isolate = info.GetIsolate();
      auto context = isolate->GetCurrentContext();

      WrappedType argWrapType = Max_WrappedType;
      if ( !util::getWrappedType( context, value, argWrapType ) || argWrapType != Wrapped_Vector3 )
      {
        isolate->ThrowException( util::staticStr( isolate, "Passed argument is not a vec3" ) );
        return;
      }

      auto object = value->ToObject( context ).ToLocalChecked();
      local_.translate_ = Vector3::unwrap( object )->shared_from_this();
    }

    void Model::js_toString( const V8CallbackArgs& args )
    {
      char result[48];
      sprintf_s<48>( result, "model[todo]" );
      args.GetReturnValue().Set( util::allocString( result, args.GetIsolate() ) );
    }

  }

}
