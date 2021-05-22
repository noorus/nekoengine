#include "stdafx.h"
#include "js_math.h"
#include "js_util.h"
#include "console.h"
#include "scripting.h"
#include "nekomath.h"
#include "js_mathutil.h"

namespace neko {

  namespace js {

    static const char* c_className = "vec3";

    static const mathShared::Messages c_equalsMessages( c_className + utf8String( ".equals" ), false );
    static const mathShared::Messages c_greaterMessages( c_className + utf8String( ".greaterThan" ), false );
    static const mathShared::Messages c_greaterOrEqualMessages( c_className + utf8String( ".greaterThanOrEqual" ), false );
    static const mathShared::Messages c_lesserMessages( c_className + utf8String( ".lessThan" ), false );
    static const mathShared::Messages c_lesserOrEqualMessages( c_className + utf8String( ".lessThanOrEqual" ), false );

    string Vector3::className( c_className );
    WrappedType Vector3::internalType = Wrapped_Vector3;

    void Vector3::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_WRAPPER_SETACCESSOR( tpl, Vector3, x, X );
      JS_WRAPPER_SETACCESSOR( tpl, Vector3, y, Y );
      JS_WRAPPER_SETACCESSOR( tpl, Vector3, z, Z );

      // Methods
      JS_WRAPPER_SETMEMBER( tpl, Vector3, toString );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, length );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, distance );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, normalise );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, normalise, normalize );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, normalisedCopy );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, normalisedCopy, normalizedCopy );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, midPoint );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, makeFloor );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, makeCeil );

      // Comparisons
      JS_WRAPPER_SETMEMBER( tpl, Vector3, equals );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, equals, eq );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, greater );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, greater, greaterThan );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, greater, gt );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, greaterOrEqual );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, greater, greaterThanOrEqual );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, greater, gte );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, lesser );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, lesser, lessThan );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, lesser, lt );
      JS_WRAPPER_SETMEMBER( tpl, Vector3, lesserOrEqual );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, lesser, lessThanOrEqual );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector3, lesser, lte );
    }

    //! \verbatim
    //! vec3()
    //! vec3( Real scalar )
    //! vec3( Real x, Real y, Real z )
    //! vec3([Real x, Real y, Real z])
    //! vec3({Real x, Real y, Real z})
    //! \endverbatim
    void Vector3::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      auto context = args.GetIsolate()->GetCurrentContext();

      vec3 vec( 0.0f, 0.0f, 0.0f );
      bool valueParsed = false;
      if ( args.Length() == 0 )
      {
        valueParsed = true;
      }
      else if ( args.Length() == 3 )
      {
        vec.x = static_cast<Real>( args[0]->NumberValue( context ).FromMaybe( 0.0 ) );
        vec.y = static_cast<Real>( args[1]->NumberValue( context ).FromMaybe( 0.0 ) );
        vec.z = static_cast<Real>( args[2]->NumberValue( context ).FromMaybe( 0.0 ) );
        valueParsed = true;
      }
      else if ( args.Length() == 1 )
      {
        if ( args[0]->IsNumber() )
        {
          auto value = static_cast<Real>( args[0]->NumberValue( context ).FromMaybe( 0.0 ) );
          vec = vec3( value, value, value );
          valueParsed = true;
        }
        else if ( args[0]->IsArray() )
        {
          auto arr = v8::Local<v8::Array>::Cast( args[0] );
          if ( arr->Length() == 3 )
          {
            vec.x = static_cast<Real>( arr->Get( 0 )->NumberValue( context ).FromMaybe( 0.0 ) );
            vec.y = static_cast<Real>( arr->Get( 1 )->NumberValue( context ).FromMaybe( 0.0 ) );
            vec.z = static_cast<Real>( arr->Get( 2 )->NumberValue( context ).FromMaybe( 0.0 ) );
            valueParsed = true;
          }
        }
        else if ( args[0]->IsObject() )
        {
          auto maybeObj = args[0]->ToObject( context );
          vec.x = util::extractNumberComponent( isolate, maybeObj, "x", 0.0f );
          vec.y = util::extractNumberComponent( isolate, maybeObj, "y", 0.0f );
          vec.z = util::extractNumberComponent( isolate, maybeObj, "z", 0.0f );
          valueParsed = true;
        }
      }

      if ( !valueParsed )
      {
        isolate->ThrowException( util::staticStr( isolate, "Invalid constructor call to vec3" ) );
        return;
      }

      auto ctx = getScriptContext( isolate );
      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr = ctx->vec3reg().createFromJS( thisObj, vec );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = ctx->vec3reg().createFrom( vec );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    //! \verbatim
    //! Real vec3.x
    //! \endverbatim
    void Vector3::js_getX( V8String prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( v_.x ) );
    }

    //! \verbatim
    //! Real vec3.x
    //! \endverbatim
    void Vector3::js_setX( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
      {
        v_.x = static_cast<Real>( value->NumberValue( context ).ToChecked() );
      }
    }

    //! \verbatim
    //! Real vec3.y
    //! \endverbatim
    void Vector3::js_getY( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( v_.y ) );
    }

    //! \verbatim
    //! Real vec3.y
    //! \endverbatim
    void Vector3::js_setY( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
      {
        v_.y = static_cast<Real>( value->NumberValue( context ).ToChecked() );
      }
    }

    //! \verbatim
    //! Real vec3.z
    //! \endverbatim
    void Vector3::js_getZ( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( v_.z ) );
    }

    //! \verbatim
    //! Real vec3.z
    //! \endverbatim
    void Vector3::js_setZ( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
      {
        v_.z = static_cast<Real>( value->NumberValue( context ).ToChecked() );
      }
    }

    //! \verbatim
    //! String vec3.toString
    //! \endverbatim
    void Vector3::js_toString( const V8CallbackArgs& args )
    {
      char result[128];
      sprintf_s<128>( result, "vec3[%f,%f,%f]", v_.x, v_.y, v_.z );
      args.GetReturnValue().Set( util::allocString( result, args.GetIsolate() ) );
    }

    //! \verbatim
    //! bool vec3.equals( vec3 )
    //! \endverbatim
    void Vector3::js_equals( const V8CallbackArgs& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 )
      {
        util::throwException( isolate, c_equalsMessages.syntaxErrorText.c_str() );
        return;
      }

      WrappedType lhsType, rhsType;
      if ( !mathShared::extractLhsRhsTypes( isolate, c_equalsMessages, args.This(), args[0], lhsType, rhsType ) )
        return;

      bool retval = mathShared::jsmath_equals( isolate, args.This(), args[0], lhsType );
      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool vec3.greaterThan( vec3 )
    //! \endverbatim
    void Vector3::js_greater( const V8CallbackArgs& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 )
      {
        util::throwException( isolate, c_greaterMessages.syntaxErrorText.c_str() );
        return;
      }

      WrappedType lhsType, rhsType;
      if ( !mathShared::extractLhsRhsTypes( isolate, c_greaterMessages, args.This(), args[0], lhsType, rhsType ) )
        return;

      bool retval = mathShared::jsmath_greater( isolate, args.This(), args[0], lhsType, false );
      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool vec3.greaterThanOrEqual( vec3 )
    //! \endverbatim
    void Vector3::js_greaterOrEqual( const V8CallbackArgs& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 )
      {
        util::throwException( isolate, c_greaterOrEqualMessages.syntaxErrorText.c_str() );
        return;
      }

      WrappedType lhsType, rhsType;
      if ( !mathShared::extractLhsRhsTypes( isolate, c_greaterOrEqualMessages, args.This(), args[0], lhsType, rhsType ) )
        return;

      bool retval = mathShared::jsmath_greater( isolate, args.This(), args[0], lhsType, true );
      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool vec3.lessThan( vec3 )
    //! \endverbatim
    void Vector3::js_lesser( const V8CallbackArgs& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 )
      {
        util::throwException( isolate, c_lesserMessages.syntaxErrorText.c_str() );
        return;
      }

      WrappedType lhsType, rhsType;
      if ( !mathShared::extractLhsRhsTypes( isolate, c_lesserMessages, args.This(), args[0], lhsType, rhsType ) )
        return;

      bool retval = mathShared::jsmath_lesser( isolate, args.This(), args[0], lhsType, false );
      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool vec3.lessThanOrEqual( vec3 )
    //! \endverbatim
    void Vector3::js_lesserOrEqual( const V8CallbackArgs& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 )
      {
        util::throwException( isolate, c_lesserOrEqualMessages.syntaxErrorText.c_str() );
        return;
      }

      WrappedType lhsType, rhsType;
      if ( !mathShared::extractLhsRhsTypes( isolate, c_lesserOrEqualMessages, args.This(), args[0], lhsType, rhsType ) )
        return;

      bool retval = mathShared::jsmath_lesser( isolate, args.This(), args[0], lhsType, true );
      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! Real vec3.length()
    //! \endverbatim
    void Vector3::js_length( const V8CallbackArgs& args )
    {
      auto length = glm::length( v_ );
      args.GetReturnValue().Set( static_cast<double>( length ) );
    }

    //! \verbatim
    //! Real vec3.distance( vec3 )
    //! \endverbatim
    void Vector3::js_distance( const V8CallbackArgs& args )
    {
      auto other = extractVector3( 0, args );
      if ( other )
      {
        auto dist = glm::distance( v_, other->v_ );
        args.GetReturnValue().Set( static_cast<double>( dist ) );
      }
    }

    //! \verbatim
    //! vec3.normalise()
    //! \endverbatim
    void Vector3::js_normalise( const V8CallbackArgs& args )
    {
      v_ = glm::normalize( v_ );
    }

    //! \verbatim
    //! vec3 vec3.normalisedCopy()
    //! \endverbatim
    void Vector3::js_normalisedCopy( const V8CallbackArgs& args )
    {
      auto normalized = glm::normalize( v_ );
      auto ctx = getScriptContext( args.GetIsolate() );
      auto ptr = ctx->vec3reg().createFrom( normalized );
      args.GetReturnValue().Set( ptr->handle( args.GetIsolate() ) );
    }

    //! \verbatim
    //! vec3 vec3.midPoint( vec3 )
    //! \endverbatim
    void Vector3::js_midPoint( const V8CallbackArgs& args )
    {
      auto other = extractVector3( 0, args );
      if ( other )
      {
        auto mid = vec3(
          ( v_.x + other->v_.x ) * 0.5f,
          ( v_.y + other->v_.y ) * 0.5f,
          ( v_.z + other->v_.z ) * 0.5f );
        auto ctx = getScriptContext( args.GetIsolate() );
        auto ptr = ctx->vec3reg().createFrom( mid );
        args.GetReturnValue().Set( ptr->handle( args.GetIsolate() ) );
      }
    }

    //! \verbatim
    //! vec3.makeFloor( vec3 )
    //! \endverbatim
    void Vector3::js_makeFloor( const V8CallbackArgs& args )
    {
      auto other = extractVector3( 0, args );
      if ( other )
      {
        if ( other->v().x < v_.x )
          v_.x = other->v().x;
        if ( other->v().y < v_.y )
          v_.y = other->v().y;
        if ( other->v().z < v_.z )
          v_.z = other->v().z;
      }
    }

    //! \verbatim
    //! vec3.makeCeil( vec3 )
    //! \endverbatim
    void Vector3::js_makeCeil( const V8CallbackArgs& args )
    {
      auto other = extractVector3( 0, args );
      if ( other )
      {
        if ( other->v().x > v_.x )
          v_.x = other->v().x;
        if ( other->v().y > v_.y )
          v_.y = other->v().y;
        if ( other->v().z > v_.z )
          v_.z = other->v().z;
      }
    }

  }

}