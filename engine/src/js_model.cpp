#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

# include "js_util.h"
# include "console.h"
# include "scripting.h"
# include "nekomath.h"
# include "js_math.h"
# include "js_mathutil.h"
# include "js_mesh.h"
# include "js_model.h"

namespace neko {

  namespace js {

    JS_DYNAMICOBJECT_DECLARE_STATICS( Model, neko::js::JSModel )

    JSModel::JSModel()
    {
      static size_t indexCounter = 0;
      id_ = indexCounter++;
    }

    void Model::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_DYNAMICOBJECT_SETACCESSOR( tpl, Model, scale, Scale );
      JS_DYNAMICOBJECT_SETACCESSOR( tpl, Model, translate, Translate );
      JS_DYNAMICOBJECT_SETACCESSOR( tpl, Model, rotate, Rotate );

      // Methods
      JS_DYNAMICOBJECT_SETMEMBER( tpl, Model, toString );
    }

    void Model::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      auto context = args.GetIsolate()->GetCurrentContext();

      const utf8String funcName( "model::Constructor" );

      vec3 defaultScale( 1.0f, 1.0f, 1.0f );
      vec3 defaultTranslate( 0.0f, 0.0f, 0.0f );
      quaternion defaultRotate( glm::quat_identity<Real, glm::defaultp>() );

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
          model.rotate_ = extractQuaternionMember( isolate, funcName, maybeObj, "rotate", false );
        }
      }

      if ( !model.mesh_ )
      {
        isolate->ThrowException( util::staticStr( isolate, "No mesh in constructor call to model" ) );
        return;
      }

      auto ctx = getScriptContext( isolate );

      if ( !model.scale_ )
        model.scale_ = ctx->vec3reg()->createFrom( defaultScale );
      if ( !model.translate_ )
        model.translate_ = ctx->vec3reg()->createFrom( defaultTranslate );
      if ( !model.rotate_ )
        model.rotate_ = ctx->quatreg()->createFrom( defaultRotate );

      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr = ctx->modelreg()->createFromJS( thisObj, model );
        ctx->renderSync().constructed( ptr.get() );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = ctx->modelreg()->createFrom( model );
        ctx->renderSync().constructed( ptr.get() );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    JS_DYNAMICOBJECT_VEC3_PROPERTY_GETSET_IMPLEMENTATIONS( Model, Scale, local_.scale_ )
    JS_DYNAMICOBJECT_VEC3_PROPERTY_GETSET_IMPLEMENTATIONS( Model, Translate, local_.translate_ )
    JS_DYNAMICOBJECT_QUATERNION_PROPERTY_GETSET_IMPLEMENTATIONS( Model, Rotate, local_.rotate_ )

    void Model::js_toString( const V8CallbackArgs& args )
    {
      char result[48];
      sprintf_s<48>( result, "model[todo]" );
      args.GetReturnValue().Set( util::allocString( result, args.GetIsolate() ) );
    }

    int32_t Model::jsEstimateSize() const
    {
      return sizeof( Model );
    }

    void Model::jsOnDestruct( Isolate* isolate )
    {
      if ( !isolate )
        return;
      auto ctx = getScriptContext( isolate );
      if ( ctx )
        ctx->renderSync().destructed( this );
    }

  }

}

#endif