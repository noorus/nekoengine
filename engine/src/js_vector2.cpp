#include "stdafx.h"
#include "js_math.h"
#include "js_util.h"
#include "console.h"
#include "scripting.h"
#include "nekomath.h"
#include "js_mathutil.h"

namespace neko {

  namespace js {

    static const char* c_className = "vec2";

    static const mathShared::Messages c_equalsMessages( c_className + utf8String( ".equals" ), false );
    static const mathShared::Messages c_greaterMessages( c_className + utf8String( ".greaterThan" ), false );
    static const mathShared::Messages c_greaterOrEqualMessages( c_className + utf8String( ".greaterThanOrEqual" ), false );
    static const mathShared::Messages c_lesserMessages( c_className + utf8String( ".lessThan" ), false );
    static const mathShared::Messages c_lesserOrEqualMessages( c_className + utf8String( ".lessThanOrEqual" ), false );
    static const mathShared::Messages c_addMessages( c_className + utf8String( ".add" ), false );
    static const mathShared::Messages c_subMessages( c_className + utf8String( ".sub" ), false );

    string Vector2::className( c_className );
    WrappedType Vector2::internalType = Wrapped_Vector2;

    void Vector2::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_WRAPPER_SETACCESSOR( tpl, Vector2, x, X );
      JS_WRAPPER_SETACCESSOR( tpl, Vector2, y, Y );

      // Operations
      JS_WRAPPER_SETMEMBER( tpl, Vector2, add );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, add, addition );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, add, plus );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, sub );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, sub, subtraction );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, sub, subtract );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, sub, minus );

      // Methods
      JS_WRAPPER_SETMEMBER( tpl, Vector2, toString );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, length );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, distance );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, dotProduct );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, crossProduct );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, normalise );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, normalise, normalize );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, normalisedCopy );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, normalisedCopy, normalizedCopy );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, midPoint );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, angleBetween );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, angleTo );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, perpendicular );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, reflect );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, makeFloor );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, makeCeil );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, inverse );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, inverse, invert );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, inverse, inverted );

      // Comparisons
      JS_WRAPPER_SETMEMBER( tpl, Vector2, equals );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, equals, eq );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, greater );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, greater, greaterThan );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, greater, gt );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, greaterOrEqual );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, greater, greaterThanOrEqual );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, greater, gte );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, lesser );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, lesser, lessThan );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, lesser, lt );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, lesserOrEqual );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, lesser, lessThanOrEqual );
      JS_WRAPPER_SETMEMBERNAMED( tpl, Vector2, lesser, lte );
    }

    //! \verbatim
    //! vec2()
    //! vec2( Real scalar )
    //! vec2( Real x, Real y )
    //! vec2([Real x, Real y])
    //! vec2({Real x, Real y})
    //! \endverbatim
    void Vector2::jsConstructor( const v8::FunctionCallbackInfo<v8::Value>& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      auto context = args.GetIsolate()->GetCurrentContext();

      vec2 vec( 0.0f, 0.0f );
      bool valueParsed = false;
      if ( args.Length() == 0 )
      {
        valueParsed = true;
      }
      else if ( args.Length() == 2 )
      {
        vec.x = static_cast<Real>( args[0]->NumberValue( context ).FromMaybe( 0.0 ) );
        vec.y = static_cast<Real>( args[1]->NumberValue( context ).FromMaybe( 0.0 ) );
        valueParsed = true;
      }
      else if ( args.Length() == 1 )
      {
        if ( args[0]->IsNumber() )
        {
          auto value = static_cast<Real>( args[0]->NumberValue( context ).FromMaybe( 0.0 ) );
          vec = vec2( value, value );
          valueParsed = true;
        }
        else if ( args[0]->IsArray() )
        {
          auto arr = v8::Local<v8::Array>::Cast( args[0] );
          if ( arr->Length() == 2 )
          {
            vec.x = static_cast<Real>( arr->Get( 0 )->NumberValue( context ).FromMaybe( 0.0 ) );
            vec.y = static_cast<Real>( arr->Get( 1 )->NumberValue( context ).FromMaybe( 0.0 ) );
            valueParsed = true;
          }
        }
        else if ( args[0]->IsObject() )
        {
          auto maybeObj = args[0]->ToObject( context );
          vec.x = util::extractNumberComponent( isolate, maybeObj, "x", 0.0f );
          vec.y = util::extractNumberComponent( isolate, maybeObj, "y", 0.0f );
          valueParsed = true;
        }
      }

      if ( !valueParsed )
      {
        isolate->ThrowException( util::staticStr( isolate, "Invalid constructor call to vec2" ) );
        return;
      }

      auto ctx = getScriptContext( isolate );
      if ( args.IsConstructCall() )
      {
        auto thisObj = args.This();
        auto ptr = ctx->vec2reg().createFromJS( thisObj, vec );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
      else
      {
        auto ptr = ctx->vec2reg().createFrom( vec );
        args.GetReturnValue().Set( ptr->handle( isolate ) );
      }
    }

    //! \verbatim
    //! Real vec2.x
    //! \endverbatim
    void Vector2::js_getX( V8String prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( v_.x ) );
    }

    //! \verbatim
    //! Real vec2.x
    //! \endverbatim
    void Vector2::js_setX( V8String prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
      {
        v_.x = static_cast<Real>( value->NumberValue( context ).ToChecked() );
      }
    }

    //! \verbatim
    //! Real vec2.y
    //! \endverbatim
    void Vector2::js_getY( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( v_.y ) );
    }

    //! \verbatim
    //! Real vec2.y
    //! \endverbatim
    void Vector2::js_setY( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
      {
        v_.y = static_cast<Real>( value->NumberValue( context ).ToChecked() );
      }
    }

    //! \verbatim
    //! vec2 vec2.add( vec2 )
    //! \endverbatim
    void Vector2::js_add( const V8CallbackArgs& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 )
      {
        util::throwException( isolate, c_addMessages.syntaxErrorText.c_str() );
        return;
      }

      mathShared::jsmath_add( args, getScriptContext( isolate ), c_addMessages, args.This(), args[0] );
    }

    //! \verbatim
    //! vec2 vec2.sub( vec2 )
    //! \endverbatim
    void Vector2::js_sub( const V8CallbackArgs& args )
    {
      auto isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 )
      {
        util::throwException( isolate, c_subMessages.syntaxErrorText.c_str() );
        return;
      }

      mathShared::jsmath_subtract( args, getScriptContext( isolate ), c_subMessages, args.This(), args[0] );
    }

    //! \verbatim
    //! String vec2.toString
    //! \endverbatim
    void Vector2::js_toString( const V8CallbackArgs& args )
    {
      char result[128];
      sprintf_s<128>( result, "vec2[%f,%f]", v_.x, v_.y );
      args.GetReturnValue().Set( util::allocString( result, args.GetIsolate() ) );
    }

    //! \verbatim
    //! bool vec2.equals( vec2 )
    //! \endverbatim
    void Vector2::js_equals( const V8CallbackArgs& args )
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
    //! bool vec2.greaterThan( vec2 )
    //! \endverbatim
    void Vector2::js_greater( const V8CallbackArgs& args )
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
    //! bool vec2.greaterThanOrEqual( vec2 )
    //! \endverbatim
    void Vector2::js_greaterOrEqual( const V8CallbackArgs& args )
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
    //! bool vec2.lessThan( vec2 )
    //! \endverbatim
    void Vector2::js_lesser( const V8CallbackArgs& args )
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
    //! bool vec2.lessThanOrEqual( vec2 )
    //! \endverbatim
    void Vector2::js_lesserOrEqual( const V8CallbackArgs& args )
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
    //! Real vec2.length()
    //! \endverbatim
    void Vector2::js_length( const V8CallbackArgs& args )
    {
      auto length = glm::length( v_ );
      args.GetReturnValue().Set( static_cast<double>( length ) );
    }

    //! \verbatim
    //! Real vec2.distance( vec2 )
    //! \endverbatim
    void Vector2::js_distance( const V8CallbackArgs& args )
    {
      auto other = extractVector2( 0, args );
      if ( other )
      {
        auto dist = glm::distance( v_, other->v_ );
        args.GetReturnValue().Set( static_cast<double>( dist ) );
      }
    }

    //! \verbatim
    //! Real vec2.dotProduct( vec2 )
    //! \endverbatim
    void Vector2::js_dotProduct( const V8CallbackArgs& args )
    {
      auto other = extractVector2( 0, args );
      if ( other )
      {
        auto product = glm::dot( v_, other->v_ );
        args.GetReturnValue().Set( static_cast<double>( product ) );
      }
    }

    //! \verbatim
    //! Real vec2.crossProduct( vec2 )
    //! \endverbatim
    void Vector2::js_crossProduct( const V8CallbackArgs& args )
    {
      auto other = extractVector2( 0, args );
      if ( other )
      {
        auto product = glm::cross( v_, other->v_ );
        args.GetReturnValue().Set( static_cast<double>( product ) );
      }
    }

    //! \verbatim
    //! vec2.normalise()
    //! \endverbatim
    void Vector2::js_normalise( const V8CallbackArgs& args )
    {
      v_ = glm::normalize( v_ );
    }

    //! \verbatim
    //! vec2 vec2.normalisedCopy()
    //! \endverbatim
    void Vector2::js_normalisedCopy( const V8CallbackArgs& args )
    {
      auto normalized = glm::normalize( v_ );
      auto ctx = getScriptContext( args.GetIsolate() );
      auto ptr = ctx->vec2reg().createFrom( normalized );
      args.GetReturnValue().Set( ptr->handle( args.GetIsolate() ) );
    }

    //! \verbatim
    //! vec2 vec2.midPoint( vec2 )
    //! \endverbatim
    void Vector2::js_midPoint( const V8CallbackArgs& args )
    {
      auto other = extractVector2( 0, args );
      if ( other )
      {
        auto mid = vec2(
          ( v_.x + other->v_.x ) * 0.5f,
          ( v_.y + other->v_.y ) * 0.5f
        );
        auto ctx = getScriptContext( args.GetIsolate() );
        auto ptr = ctx->vec2reg().createFrom( mid );
        args.GetReturnValue().Set( ptr->handle( args.GetIsolate() ) );
      }
    }

    //! \verbatim
    //! Radian vec2.angleBetween( vec2 )
    //! \endverbatim
    void Vector2::js_angleBetween( const V8CallbackArgs& args )
    {
      auto other = extractVector2( 0, args );
      if ( other )
      {
        auto angle = math::angleBetween( v_, other->v() );
        args.GetReturnValue().Set( static_cast<double>( angle ) );
      }
    }

    //! \verbatim
    //! Radian vec2.angleTo( vec2 )
    //! \endverbatim
    void Vector2::js_angleTo( const V8CallbackArgs& args )
    {
      auto other = extractVector2( 0, args );
      if ( other )
      {
        auto angle = math::angleBetween( v_, other->v() );
        if ( glm::cross( v_, other->v() ) < glm::zero<Real>() )
          angle = ( glm::two_pi<Real>() - angle );
        args.GetReturnValue().Set( static_cast<double>( angle ) );
      }
    }

    //! \verbatim
    //! vec2 vec2.perpendicular()
    //! \endverbatim
    void Vector2::js_perpendicular( const V8CallbackArgs& args )
    {
      auto ctx = getScriptContext( args.GetIsolate() );
      auto ptr = ctx->vec2reg().createFrom( vec2( -v_.y, v_.x ) );
      args.GetReturnValue().Set( ptr->handle( args.GetIsolate() ) );
    }

    //! \verbatim
    //! vec2 vec2.reflect( vec2 )
    //! \endverbatim
    void Vector2::js_reflect( const V8CallbackArgs& args )
    {
      auto normal = extractVector2( 0, args );
      if ( normal )
      {
        auto ret = vec2( v_ - ( 2 * glm::dot( v_, normal->v() ) * normal->v() ) );
        auto ctx = getScriptContext( args.GetIsolate() );
        auto ptr = ctx->vec2reg().createFrom( ret );
        args.GetReturnValue().Set( ptr->handle( args.GetIsolate() ) );
      }
    }

    //! \verbatim
    //! vec2.makeFloor( vec2 )
    //! \endverbatim
    void Vector2::js_makeFloor( const V8CallbackArgs& args )
    {
      auto other = extractVector2( 0, args );
      if ( other )
      {
        if ( other->v().x < v_.x )
          v_.x = other->v().x;
        if ( other->v().y < v_.y )
          v_.y = other->v().y;
      }
    }

    //! \verbatim
    //! vec2.makeCeil( vec2 )
    //! \endverbatim
    void Vector2::js_makeCeil( const V8CallbackArgs& args )
    {
      auto other = extractVector2( 0, args );
      if ( other )
      {
        if ( other->v().x > v_.x )
          v_.x = other->v().x;
        if ( other->v().y > v_.y )
          v_.y = other->v().y;
      }
    }

    //! \verbatim
    //! vec2 vec2.inverse()
    //! \endverbatim
    void Vector2::js_inverse( const V8CallbackArgs& args )
    {
      auto ctx = getScriptContext( args.GetIsolate() );
      auto ptr = ctx->vec2reg().createFrom( vec2( -v_.x, -v_.y ) );
      args.GetReturnValue().Set( ptr->handle( args.GetIsolate() ) );
    }

  }

}