#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

# include "js_util.h"
# include "console.h"
# include "scripting.h"
# include "nekomath.h"
# include "js_math.h"
# include "js_mathutil.h"
# include "js_entity.h"
# include "locator.h"
# include "console.h"

namespace neko {

  namespace js {

    static const char* c_className = "entity";

    string DynamicObjectWrapper<Entity, JSEntity>::className( c_className );
    WrappedType DynamicObjectWrapper<Entity, JSEntity>::internalType = Wrapped_Entity;

    void Entity::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_WRAPPER_SETACCESSOR( tpl, Entity, scale, Scale );
      JS_WRAPPER_SETACCESSOR( tpl, Entity, translate, Translate );
      JS_WRAPPER_SETACCESSOR( tpl, Entity, rotate, Rotate );

      // Methods
      JS_WRAPPER_SETMEMBER( tpl, Entity, toString );
    }

    void Entity::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      auto context = args.GetIsolate()->GetCurrentContext();

      const utf8String funcName( "entity::Constructor" );

      vec3 defaultScale( 1.0f, 1.0f, 1.0f );
      vec3 defaultTranslate( 0.0f, 0.0f, 0.0f );
      quaternion defaultRotate( glm::quat_identity<Real, glm::defaultp>() );

      JSEntity loc;

      WrappedType wrappedType = Max_WrappedType;
      if ( args.Length() == 1 )
      {
        util::getWrappedType( context, args[0], wrappedType );
        if ( args[0]->IsObject() && wrappedType == Max_WrappedType )
        {
          auto maybeObj = args[0]->ToObject( context );
          loc.scale = extractVector3Member( isolate, funcName, maybeObj, "scale", false );
          loc.translate = extractVector3Member( isolate, funcName, maybeObj, "translate", false );
          loc.rotate = extractQuaternionMember( isolate, funcName, maybeObj, "rotate", false );
        }
      }

      auto ctx = getScriptContext( isolate );

      if ( !loc.scale )
        loc.scale = ctx->vec3reg().createFrom( defaultScale );
      if ( !loc.translate )
        loc.translate = ctx->vec3reg().createFrom( defaultTranslate );
      if ( !loc.rotate )
        loc.rotate = ctx->quatreg().createFrom( defaultRotate );

      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr = ctx->entreg().createFromJS( thisObj, loc );
        //ctx->renderSync().constructed( ptr.get() );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = ctx->entreg().createFrom( loc );
        //ctx->renderSync().constructed( ptr.get() );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    JS_WRAPPER_VEC3_PROPERTY_GETSET_IMPLEMENTATIONS( Entity, Scale, local_.scale )
    JS_WRAPPER_VEC3_PROPERTY_GETSET_IMPLEMENTATIONS( Entity, Translate, local_.translate )
    JS_WRAPPER_QUATERNION_PROPERTY_GETSET_IMPLEMENTATIONS( Entity, Rotate, local_.rotate )

    void Entity::js_toString( const V8CallbackArgs& args )
    {
      args.GetReturnValue().Set( util::allocString( utils::ilprinf( "entity[%i]", local_.id ), args.GetIsolate() ) );
    }

    int32_t Entity::jsEstimateSize() const
    {
      int64_t result = sizeof( Text ) + sizeof( js::Vector3 ) + sizeof( js::Vector3 ) + sizeof( js::Quaternion );
      // add in the size of the mesh and whatever in bytes maybe
      return static_cast<int32_t>( std::min( static_cast<int64_t>( std::numeric_limits<int32_t>::max() ), result ) );
    }

    void Entity::jsOnDestruct( Isolate* isolate )
    {
      if ( !isolate )
        return;
      auto ctx = getScriptContext( isolate );
    }

  }

}

#endif