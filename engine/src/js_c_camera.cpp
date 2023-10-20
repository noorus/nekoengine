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

    JS_MAPPEDDYNAMICOBJECT_DECLARE_STATICS( CameraComponent, JSCameraComponent )

    void CameraComponent::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_DYNAMICOBJECT_SETACCESSOR( tpl, CameraComponent, projection, Projection );
      JS_DYNAMICOBJECT_SETACCESSOR( tpl, CameraComponent, near, Near );
      JS_DYNAMICOBJECT_SETACCESSOR( tpl, CameraComponent, far, Far );
      JS_DYNAMICOBJECT_SETACCESSOR( tpl, CameraComponent, exposure, Exposure );

      // Methods
      JS_DYNAMICOBJECT_SETMEMBER( tpl, CameraComponent, toString );
    }

    void CameraComponent::js_getProjection( V8String prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      if ( local_.isOrtho )
        info.GetReturnValue().Set( util::utf8Literal( info.GetIsolate(), "orthographic" ) );
      else
        info.GetReturnValue().Set( util::utf8Literal( info.GetIsolate(), "perspective" ) );
      js_afterGetProjection( info );
    }

    void CameraComponent::js_setProjection( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto isolate = info.GetIsolate();
      auto context = isolate->GetCurrentContext();
      if ( !value->IsString() )
      {
        isolate->ThrowException( util::staticStr( isolate, "Passed value is not a string" ) );
        return;
      }
      local_.isOrtho = ( _stricmp( *( v8::String::Utf8Value( isolate, value ) ), "orthographic" ) );
      js_afterSetProjection( info );
    }

    void CameraComponent::js_getNear( V8String prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( local_.nearDist );
      js_afterGetNear( info );
    }

    void CameraComponent::js_setNear( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto isolate = info.GetIsolate();
      auto context = isolate->GetCurrentContext();
      if ( !value->IsNumber() )
      {
        isolate->ThrowException( util::staticStr( isolate, "Passed value is not a number" ) );
        return;
      }
      local_.nearDist = static_cast<Real>( value->NumberValue( context ).ToChecked() );
      js_afterSetNear( info );
    }

    void CameraComponent::js_getFar( V8String prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( local_.farDist );
      js_afterGetFar( info );
    }

    void CameraComponent::js_setFar( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto isolate = info.GetIsolate();
      auto context = isolate->GetCurrentContext();
      if ( !value->IsNumber() )
      {
        isolate->ThrowException( util::staticStr( isolate, "Passed value is not a number" ) );
        return;
      }
      local_.farDist = static_cast<Real>( value->NumberValue( context ).ToChecked() );
      js_afterSetFar( info );
    }

    void CameraComponent::js_getExposure( V8String prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( local_.exposure );
      js_afterGetExposure( info );
    }

    void CameraComponent::js_setExposure( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto isolate = info.GetIsolate();
      auto context = isolate->GetCurrentContext();
      if ( !value->IsNumber() )
      {
        isolate->ThrowException( util::staticStr( isolate, "Passed value is not a number" ) );
        return;
      }
      local_.exposure = static_cast<Real>( value->NumberValue( context ).ToChecked() );
      js_afterSetExposure( info );
    }

    void CameraComponent::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      JS_DYNAMICOBJECT_CONSTRUCTBODY_BEGIN( CameraComponent )

      JSCameraComponent loc( 0xFFFFFFFF );
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
          auto near = util::extractNumberMember( isolate, funcName, maybeObj, "near", false );
          if ( near )
            loc.nearDist = *near;
          auto far = util::extractNumberMember( isolate, funcName, maybeObj, "far", false );
          if ( far )
            loc.farDist = *far;
          auto exp = util::extractNumberMember( isolate, funcName, maybeObj, "exposure", false );
          if ( exp )
            loc.exposure = *exp;
        }
        else if ( args[0]->IsNumber() && wrappedType == Max_WrappedType )
        {
          auto eid = util::uint32FromValue( context, args[0], 0, true );
          loc.eid = static_cast<c::entity>( eid );
        }
      }

      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr = scriptCtx->registry<CameraComponent>( type<CameraComponent> {} )
                     ->createFromJS( thisObj, static_cast<uint64_t>( loc.eid ), loc );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = scriptCtx->registry<CameraComponent>( type<CameraComponent> {} )
                     ->createFrom( static_cast<uint64_t>( loc.eid ), loc );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    void CameraComponent::js_afterSetProjection( const PropertyCallbackInfo<void>& info )
    {
      auto ctx = scriptContext( info.GetIsolate() );
      auto& cam = ctx->scene().cam( local_.eid );
      cam.projection = ( local_.isOrtho ? c::camera::Orthographic : c::camera::Perspective );
    }

    void CameraComponent::js_afterSetNear( const PropertyCallbackInfo<void>& info )
    {
      auto ctx = scriptContext( info.GetIsolate() );
      auto& cam = ctx->scene().cam( local_.eid );
      cam.nearDist = local_.nearDist;
    }

    void CameraComponent::js_afterSetFar( const PropertyCallbackInfo<void>& info )
    {
      auto ctx = scriptContext( info.GetIsolate() );
      auto& cam = ctx->scene().cam( local_.eid );
      cam.farDist = local_.farDist;
    }

    void CameraComponent::js_afterSetExposure( const PropertyCallbackInfo<void>& info )
    {
      auto ctx = scriptContext( info.GetIsolate() );
      auto& cam = ctx->scene().cam( local_.eid );
      cam.exposure = local_.exposure;
    }

    void CameraComponent::js_toString( const V8CallbackArgs& args )
    {
      args.GetReturnValue().Set( util::allocString(
        utils::ilprinf( "camera[%i,%.3f,%.3f,%.2f]", local_.eid, local_.nearDist, local_.farDist, local_.exposure ),
        args.GetIsolate() ) );
    }

    int32_t CameraComponent::jsEstimateSize() const
    {
      int64_t result = sizeof( JSCameraComponent );
      return static_cast<int32_t>( std::min( static_cast<int64_t>( std::numeric_limits<int32_t>::max() ), result ) );
    }

    void CameraComponent::jsOnDestruct( Isolate* isolate )
    {
      if ( !isolate )
        return;
      auto ctx = getScriptContext( isolate );
    }

  }

}

#endif