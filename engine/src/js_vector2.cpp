#include "stdafx.h"
#include "js_math.h"
#include "js_util.h"
#include "console.h"

namespace neko {

  namespace js {

    string Vector2::className( "Vector2" );
    WrappedType Vector2::internalType = Wrapped_Vector2;

    Vector2::Vector2( const vec2& source ): inner_( source )
    {
    }

    Vector2::~Vector2()
    {
      OutputDebugStringA( "VECTOR2 DESTRUCTOR!\r\n" );
      DebugBreak();
    }

    void Vector2::registerExport( Isolate* isolate, v8::Local<v8::FunctionTemplate>& obj )
    {
      JS_WRAPPER_SETACCESSOR( obj, Vector2, x, X );
      JS_WRAPPER_SETACCESSOR( obj, Vector2, y, Y );
      JS_WRAPPER_SETINTMEMBER( obj, Vector2, toString );
    }

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
      if ( args.Length() == 2 )
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
          vec.x = util::extractNumberComponent( isolate, args[0]->ToObject( context ), "x", 0.0f );
          vec.y = util::extractNumberComponent( isolate, args[0]->ToObject( context ), "y", 0.0f );
          valueParsed = true;
        }
      }

      if ( !valueParsed )
      {
        args.GetIsolate()->ThrowException( util::allocString( "Invalid constructor call to Vector2" ) );
        return;
      }

      auto retval = new Vector2( vec );
      retval->wrapperWrap( args.This() );
      args.GetReturnValue().Set( args.This() );
    }

    void Vector2::js_getX( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( inner_.x ) );
    }

    void Vector2::js_setX( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
      {
        inner_.x = static_cast<Real>( value->NumberValue( context ).ToChecked() );
      }
    }

    void Vector2::js_getY( Local<v8::String> prop, const PropertyCallbackInfo<v8::Value>& info )
    {
      info.GetReturnValue().Set( static_cast<double>( inner_.y ) );
    }

    void Vector2::js_setY( Local<v8::String> prop, Local<v8::Value> value, const PropertyCallbackInfo<void>& info )
    {
      auto context = info.GetIsolate()->GetCurrentContext();
      if ( value->IsNumber() && !value->NumberValue( context ).IsNothing() )
      {
        inner_.y = static_cast<Real>( value->NumberValue( context ).ToChecked() );
      }
    }

    void Vector2::js_toString( const V8CallbackArgs& args )
    {
      char result[128];
      sprintf_s<128>( result, "Vector2[%f,%f]", inner_.x, inner_.y );
      args.GetReturnValue().Set( util::allocString( result, args.GetIsolate() ) );
    }

  }

}