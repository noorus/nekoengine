#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

# include "js_util.h"
# include "console.h"
# include "scripting.h"
# include "nekomath.h"
# include "js_math.h"
# include "js_mathutil.h"
# include "js_entity.h"
# include "js_util.h"
# include "js_component.h"
# include "locator.h"
# include "console.h"

namespace neko {

  namespace js {

    JS_MAPPEDDYNAMICOBJECT_DECLARE_STATICS( TransformComponent, JSTransformComponent )

    void TransformComponent::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_DYNAMICOBJECT_SETACCESSOR( tpl, TransformComponent, scale, Scale );
      JS_DYNAMICOBJECT_SETACCESSOR( tpl, TransformComponent, translate, Translate );
      JS_DYNAMICOBJECT_SETACCESSOR( tpl, TransformComponent, rotate, Rotate );

      // Methods
      JS_DYNAMICOBJECT_SETMEMBER( tpl, TransformComponent, toString );
    }

    JS_DYNAMICOBJECT_VEC3_PROPERTY_GETSET_IMPLEMENTATIONS_WITH_CALLBACKS( TransformComponent, Scale, local_.scale )
    JS_DYNAMICOBJECT_VEC3_PROPERTY_GETSET_IMPLEMENTATIONS_WITH_CALLBACKS( TransformComponent, Translate, local_.translate )
    JS_DYNAMICOBJECT_QUATERNION_PROPERTY_GETSET_IMPLEMENTATIONS_WITH_CALLBACKS( TransformComponent, Rotate, local_.rotate )

    void TransformComponent::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      JS_DYNAMICOBJECT_CONSTRUCTBODY_BEGIN( TransformComponent )

      vec3 defaultScale( 1.0f, 1.0f, 1.0f );
      vec3 defaultTranslate( 0.0f, 0.0f, 0.0f );
      quaternion defaultRotate( glm::quat_identity<Real, glm::defaultp>() );

      JSTransformComponent loc( 0xFFFFFFFF );
      WrappedType wrappedType = Max_WrappedType;
      if ( args.Length() == 1 )
      {
        util::getWrappedType( context, args[0], wrappedType );
        if ( args[0]->IsObject() && wrappedType == Max_WrappedType )
        {
          auto maybeObj = args[0]->ToObject( context );
          auto eid = util::extractUInt32Member( isolate, funcName, maybeObj, "entity", true );
          if ( !eid )
            NEKO_EXCEPT( "No valid entity ID passed" );
          loc.eid = static_cast<c::entity>( *eid );
          loc.scale = extractVector3Member( isolate, funcName, maybeObj, "scale", false );
          loc.translate = extractVector3Member( isolate, funcName, maybeObj, "translate", false );
          loc.rotate = extractQuaternionMember( isolate, funcName, maybeObj, "rotate", false );
        }
        else if ( args[0]->IsNumber() && wrappedType == Max_WrappedType )
        {
          auto eid = util::uint32FromValue( context, args[0], 0, true );
          loc.eid = static_cast<c::entity>( eid );
        }
      }

      if ( !loc.scale )
        loc.scale = scriptCtx->vec3reg()->createFrom( defaultScale );
      if ( !loc.translate )
        loc.translate = scriptCtx->vec3reg()->createFrom( defaultTranslate );
      if ( !loc.rotate )
        loc.rotate = scriptCtx->quatreg()->createFrom( defaultRotate );

      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr =
          scriptCtx->registry<TransformComponent>( type<TransformComponent> {} )->createFromJS( thisObj, static_cast<uint64_t>( loc.eid ), loc );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = scriptCtx->registry<TransformComponent>( type<TransformComponent> {} )->createFrom( static_cast<uint64_t>( loc.eid ), loc );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    void TransformComponent::js_afterSetScale( const PropertyCallbackInfo<void>& info )
    {
      auto ctx = scriptContext( info.GetIsolate() );
      auto& node = ctx->scene().tn( local_.eid );
      node.scale = local_.scale->v();
    }

    void TransformComponent::js_afterSetTranslate( const PropertyCallbackInfo<void>& info )
    {
      auto ctx = scriptContext( info.GetIsolate() );
      auto& node = ctx->scene().tn( local_.eid );
      node.translate = local_.translate->v();
    }

    void TransformComponent::js_afterSetRotate( const PropertyCallbackInfo<void>& info )
    {
      auto ctx = scriptContext( info.GetIsolate() );
      auto& node = ctx->scene().tn( local_.eid );
      node.rotate = local_.rotate->q();
    }

    void TransformComponent::js_toString( const V8CallbackArgs& args )
    {
      args.GetReturnValue().Set( util::allocString( utils::ilprintf( "component[%i]", local_.eid ), args.GetIsolate() ) );
    }

    int32_t TransformComponent::jsEstimateSize() const
    {
      int64_t result = sizeof( JSTransformComponent ) + sizeof( js::Vector3 ) + sizeof( js::Vector3 ) + sizeof( js::Quaternion );
      return static_cast<int32_t>( std::min( static_cast<int64_t>( std::numeric_limits<int32_t>::max() ), result ) );
    }

    void TransformComponent::jsOnDestruct( Isolate* isolate )
    {
      if ( !isolate )
        return;
      auto ctx = getScriptContext( isolate );
    }

  }

}

#endif