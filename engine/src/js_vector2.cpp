#include "stdafx.h"
#include "js_math.h"
#include "js_util.h"
#include "console.h"
#include "scripting.h"
#include "nekomath.h"

namespace neko {

  namespace js {

    string Vector2::className( "vec2" );
    WrappedType Vector2::internalType = Wrapped_Vector2;

    void Vector2::registerExport( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      // Properties
      JS_WRAPPER_SETACCESSOR( tpl, Vector2, x, X );
      JS_WRAPPER_SETACCESSOR( tpl, Vector2, y, Y );

      // Methods
      JS_WRAPPER_SETMEMBER( tpl, Vector2, toString );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, length );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, distance );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, dotProduct );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, crossProduct );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, normalise );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, normalisedCopy );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, midPoint );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, angleBetween );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, angleTo );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, perpendicular );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, reflect );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, makeFloor );
      JS_WRAPPER_SETMEMBER( tpl, Vector2, makeCeil );
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

      if ( !args.IsConstructCall() )
      {
        args.GetIsolate()->ThrowException( util::allocString( "Function called as non-constructor" ) );
        return;
      }

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
        args.GetIsolate()->ThrowException( util::allocString( "Invalid constructor call to vec2" ) );
        return;
      }

      auto ctx = getScriptContext( args.GetIsolate() );
      auto thisObj = args.This();
      auto ptr = ctx->vec2reg().createFromJS( thisObj, vec );
      args.GetReturnValue().Set( ptr->handle( isolate ) );
    }

    //! \verbatim
    //! Real vec2.x
    //! \endverbatim
    void Vector2::js_getX( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( v_.x ) );
    }

    //! \verbatim
    //! Real vec2.x
    //! \endverbatim
    void Vector2::js_setX( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
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
    //! String vec2.toString
    //! \endverbatim
    void Vector2::js_toString( const V8CallbackArgs& args )
    {
      char result[128];
      sprintf_s<128>( result, "vec2[%f,%f]", v_.x, v_.y );
      args.GetReturnValue().Set( util::allocString( result, args.GetIsolate() ) );
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

    inline Real angleBetween( vec2& v1, vec2& v2 )
    {
      auto lenProd = glm::length( v1 ) * glm::length( v2 );
      if ( lenProd < 1e-6f )
        lenProd = 1e-6f;
      auto f = ( glm::dot( v1, v2 ) / lenProd );
      return math::acos( math::clamp( f, -(glm::one<Real>()), glm::one<Real>() ) );
    }

    //! \verbatim
    //! Radian vec2.angleBetween( vec2 )
    //! \endverbatim
    void Vector2::js_angleBetween( const V8CallbackArgs& args )
    {
      auto other = extractVector2( 0, args );
      if ( other )
      {
        auto angle = angleBetween( v_, other->v() );
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
        auto angle = angleBetween( v_, other->v() );
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

  }

}