#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

# include "js_math.h"
# include "js_util.h"
# include "console.h"
# include "scripting.h"
# include "nekomath.h"
# include "js_mathutil.h"

namespace neko {

  namespace js {

    JS_DYNAMICOBJECT_DECLARE_STATICS_NAMED( Quaternion, neko::quaternion, quat )

    void Quaternion::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_WRAPPER_SETACCESSOR( tpl, Quaternion, x, X );
      JS_WRAPPER_SETACCESSOR( tpl, Quaternion, y, Y );
      JS_WRAPPER_SETACCESSOR( tpl, Quaternion, z, Z );
      JS_WRAPPER_SETACCESSOR( tpl, Quaternion, w, W );

      // Methods
      JS_WRAPPER_SETMEMBER( tpl, Quaternion, toString );
      JS_WRAPPER_SETMEMBER( tpl, Quaternion, length );
      JS_WRAPPER_SETMEMBER( tpl, Quaternion, dotProduct );
      JS_WRAPPER_SETMEMBER( tpl, Quaternion, normalise );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Quaternion, normalise, normalize );
      JS_WRAPPER_SETMEMBER( tpl, Quaternion, normalisedCopy );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Quaternion, normalisedCopy, normalizedCopy );
      JS_WRAPPER_SETMEMBER( tpl, Quaternion, lerp );
      JS_WRAPPER_SETMEMBER( tpl, Quaternion, slerp );
      JS_WRAPPER_SETMEMBER( tpl, Quaternion, fromAngleAxis );
    }

    //! \verbatim
    //! quaternion()
    //! quaternion( Real scalar )
    //! quaternion( Real x, Real y, Real z, Real w )
    //! quaternion([Real x, Real y, Real z, Real w])
    //! quaternion({Real x, Real y, Real z, Real w})
    //! \endverbatim
    void Quaternion::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      auto context = args.GetIsolate()->GetCurrentContext();

      quaternion quat = glm::quat_identity<Real, glm::defaultp>();
      bool valueParsed = false;
      if ( args.Length() == 0 )
      {
        valueParsed = true;
      }
      else if ( args.Length() == 4 )
      {
        quat.x = static_cast<Real>( args[0]->NumberValue( context ).FromMaybe( 0.0 ) );
        quat.y = static_cast<Real>( args[1]->NumberValue( context ).FromMaybe( 0.0 ) );
        quat.z = static_cast<Real>( args[2]->NumberValue( context ).FromMaybe( 0.0 ) );
        quat.w = static_cast<Real>( args[3]->NumberValue( context ).FromMaybe( 0.0 ) );
        valueParsed = true;
      }
      else if ( args.Length() == 1 )
      {
        if ( args[0]->IsArray() )
        {
          auto arr = v8::Local<v8::Array>::Cast( args[0] );
          if ( arr->Length() == 4 )
          {
            quat.x = static_cast<Real>( arr->Get( context, 0 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
            quat.y = static_cast<Real>( arr->Get( context, 1 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
            quat.z = static_cast<Real>( arr->Get( context, 2 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
            quat.w = static_cast<Real>( arr->Get( context, 3 ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
            valueParsed = true;
          }
        }
        else if ( args[0]->IsObject() )
        {
          auto maybeObj = args[0]->ToObject( context );
          quat.x = util::extractNumberComponent( isolate, maybeObj, "x", 0.0f );
          quat.y = util::extractNumberComponent( isolate, maybeObj, "y", 0.0f );
          quat.z = util::extractNumberComponent( isolate, maybeObj, "z", 0.0f );
          quat.w = util::extractNumberComponent( isolate, maybeObj, "w", 0.0f );
          valueParsed = true;
        }
      }

      if ( !valueParsed )
      {
        isolate->ThrowException( util::staticStr( isolate, "Invalid constructor call to quaternion" ) );
        return;
      }

      auto ctx = getScriptContext( isolate );
      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
       // ctx->registry<q
        auto ptr = ctx->quatreg()->createFromJS( thisObj, quat );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = ctx->quatreg()->createFrom( quat );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    //! \verbatim
    //! Real quaternion.x
    //! \endverbatim
    void Quaternion::js_getX( V8String prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( q_.x ) );
    }

    //! \verbatim
    //! Real quaternion.x
    //! \endverbatim
    void Quaternion::js_setX( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
        q_.x = static_cast<Real>( value->NumberValue( context ).ToChecked() );
    }

    //! \verbatim
    //! Real quaternion.y
    //! \endverbatim
    void Quaternion::js_getY( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( q_.y ) );
    }

    //! \verbatim
    //! Real quaternion.y
    //! \endverbatim
    void Quaternion::js_setY( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
        q_.y = static_cast<Real>( value->NumberValue( context ).ToChecked() );
    }

    //! \verbatim
    //! Real quaternion.z
    //! \endverbatim
    void Quaternion::js_getZ( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( q_.z ) );
    }

    //! \verbatim
    //! Real quaternion.z
    //! \endverbatim
    void Quaternion::js_setZ( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
        q_.z = static_cast<Real>( value->NumberValue( context ).ToChecked() );
    }

    //! \verbatim
    //! Real quaternion.w
    //! \endverbatim
    void Quaternion::js_getW( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( q_.w ) );
    }

    //! \verbatim
    //! Real quaternion.w
    //! \endverbatim
    void Quaternion::js_setW( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
        q_.w = static_cast<Real>( value->NumberValue( context ).ToChecked() );
    }

    //! \verbatim
    //! String quaternion.toString
    //! \endverbatim
    void Quaternion::js_toString( const V8CallbackArgs& args )
    {
      char result[128];
      sprintf_s<128>( result, "quaternion[%f,%f,%f,%f]", q_.x, q_.y, q_.z, q_.w );
      args.GetReturnValue().Set( util::allocString( result, args.GetIsolate() ) );
    }

    //! \verbatim
    //! Real quaternion.length()
    //! \endverbatim
    void Quaternion::js_length( const V8CallbackArgs& args )
    {
      auto length = glm::length( q_ );
      args.GetReturnValue().Set( static_cast<double>( length ) );
    }

    //! \verbatim
    //! Real quaternion.dotProduct( quaternion )
    //! \endverbatim
    void Quaternion::js_dotProduct( const V8CallbackArgs& args )
    {
      auto other = extractQuaternion( 0, args );
      if ( other )
      {
        auto product = glm::dot( q_, other->q_ );
        args.GetReturnValue().Set( static_cast<double>( product ) );
      }
    }

    //! \verbatim
    //! quaternion.normalise()
    //! \endverbatim
    void Quaternion::js_normalise( const V8CallbackArgs& args )
    {
      q_ = glm::normalize( q_ );
    }

    //! \verbatim
    //! quaternion quaternion.normalisedCopy()
    //! \endverbatim
    void Quaternion::js_normalisedCopy( const V8CallbackArgs& args )
    {
      auto normalized = glm::normalize( q_ );
      auto ctx = getScriptContext( args.GetIsolate() );
      auto ptr = ctx->quatreg()->createFrom( normalized );
      args.GetReturnValue().Set( ptr->handle( args.GetIsolate() ) );
    }

    //! \verbatim
    //! quaternion quaternion.lerp( quaternion, interp )
    //! \endverbatim
    void Quaternion::js_lerp( const V8CallbackArgs& args )
    {
      auto isolate = args.GetIsolate();
      if ( args.Length() != 2 || !args[1]->IsNumber() )
      {
        isolate->ThrowException( util::staticStr( isolate, "Expected arguments quaternion, interp" ) );
        return;
      }
      auto context = isolate->GetCurrentContext();
      auto other = extractQuaternion( 0, args );
      auto interp = math::clamp( (Real)args[1]->NumberValue( context ).ToChecked(), 0.0f, 1.0f );
      if ( !other )
      {
        isolate->ThrowException( util::staticStr( isolate, "Expected arguments quaternion, interp" ) );
        return;
      }
      auto resQ = glm::lerp( q_, other->q_, interp );
      auto ctx = getScriptContext( args.GetIsolate() );
      auto ptr = ctx->quatreg()->createFrom( resQ );
      args.GetReturnValue().Set( ptr->handle( args.GetIsolate() ) );
    }

    //! \verbatim
    //! quaternion quaternion.slerp( quaternion, interp )
    //! \endverbatim
    void Quaternion::js_slerp( const V8CallbackArgs& args )
    {
      auto isolate = args.GetIsolate();
      if ( args.Length() != 2 || !args[1]->IsNumber() )
      {
        isolate->ThrowException( util::staticStr( isolate, "Expected arguments quaternion, interp" ) );
        return;
      }
      auto context = isolate->GetCurrentContext();
      auto other = extractQuaternion( 0, args );
      auto interp = math::clamp( (Real)args[1]->NumberValue( context ).ToChecked(), 0.0f, 1.0f );
      if ( !other )
      {
        isolate->ThrowException( util::staticStr( isolate, "Expected arguments quaternion, interp" ) );
        return;
      }
      auto resQ = glm::slerp( q_, other->q_, interp );
      auto ctx = getScriptContext( args.GetIsolate() );
      auto ptr = ctx->quatreg()->createFrom( resQ );
      args.GetReturnValue().Set( ptr->handle( args.GetIsolate() ) );
    }

    void Quaternion::js_fromAngleAxis( const V8CallbackArgs& args )
    {
      auto isolate = args.GetIsolate();
      if ( args.Length() != 2 || !args[0]->IsNumber() )
      {
        isolate->ThrowException( util::staticStr( isolate, "Expected arguments angle, vec3" ) );
        return;
      }
      auto context = isolate->GetCurrentContext();
      auto angle = (Real)args[0]->NumberValue( context ).ToChecked();
      auto axis = extractVector3( 1, args );
      if ( !axis )
      {
        isolate->ThrowException( util::staticStr( isolate, "Expected arguments angle, vec3" ) );
        return;
      }
      q_ = glm::angleAxis( angle, axis->v() );
    }

  }

}

#endif