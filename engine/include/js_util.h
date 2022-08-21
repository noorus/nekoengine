#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "neko_exception.h"

#include <v8.h>

namespace neko {

  namespace js {

    using v8::Isolate;
    using v8::HandleScope;
    using v8::Local;
    using v8::Persistent;
    using v8::Eternal;
    using v8::Global;
    using v8::PropertyCallbackInfo;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;
    using v8::FunctionCallbackInfo;
    using v8::EscapableHandleScope;

    using V8CallbackArgs = v8::FunctionCallbackInfo<v8::Value>;
    using V8FunctionTemplate = v8::Local<v8::FunctionTemplate>;
    using V8Object = v8::Local<v8::Object>;
    using V8Value = v8::Local<v8::Value>;
    using V8Context = v8::Local<v8::Context>;
    using V8Function = v8::Local<v8::Function>;
    using V8String = v8::Local<v8::String>;

    namespace util {

#     define JS_TEMPLATE_ACCESSOR(tpl,x,y,z) tpl->PrototypeTemplate()->SetAccessor( \
        util::allocString( x ), y, z )

      inline V8String staticStr( Isolate* isolate, const char* data )
      {
        return v8::String::NewFromOneByte( isolate,
          reinterpret_cast<const uint8_t*>( data ),
          v8::NewStringType::kInternalized, -1 ).ToLocalChecked();
      }

      inline V8String staticStr( Isolate* isolate, const char* data, int length )
      {
        return v8::String::NewFromOneByte( isolate,
          reinterpret_cast<const uint8_t*>( data ),
          v8::NewStringType::kInternalized, length ).ToLocalChecked();
      }

      //! Create a local JavaScript String instance from given source string.
      inline V8String allocString( const utf8String& str, Isolate* isolate = nullptr )
      {
        v8::MaybeLocal<v8::String> ret = v8::String::NewFromUtf8(
          isolate ? isolate : v8::Isolate::GetCurrent(), str.c_str(), v8::NewStringType::kNormal );
        if ( ret.IsEmpty() )
          NEKO_EXCEPT( "V8 String allocation failed" );
        return ret.ToLocalChecked();
      }

      //! Create a local JavaScript String instance from given source string.
      inline V8String allocString( const wstring& str, Isolate* isolate = nullptr )
      {
        v8::MaybeLocal<v8::String> ret = v8::String::NewFromTwoByte(
          isolate ? isolate : v8::Isolate::GetCurrent(), (uint16_t*)str.c_str(), v8::NewStringType::kNormal );
        if ( ret.IsEmpty() )
          NEKO_EXCEPT( "V8 String allocation failed" );
        return ret.ToLocalChecked();
      }

      //! Create a local V8 String that will likely be duplicated (class names etc.)
      inline V8String allocStringConserve( string_view str, Isolate* isolate = nullptr )
      {
        auto ret = v8::String::NewFromUtf8( isolate ? isolate : Isolate::GetCurrent(), str.data(), v8::NewStringType::kInternalized );
        if ( ret.IsEmpty() )
          NEKO_EXCEPT( "V8 String allocation failed" );
        return ret.ToLocalChecked();
      }

      inline void throwException( Isolate* isolate, const char* message )
      {
        isolate->ThrowException( staticStr( isolate, message ) );
      }

      inline void throwException( Isolate* isolate, const wchar_t* message, ... )
      {
        wchar_t buffer[1024];

        va_list va_alist;
        va_start( va_alist, message );
        _vsnwprintf_s( buffer, 1024, message, va_alist );
        va_end( va_alist );

        isolate->ThrowException( allocString( buffer, isolate ) );
      }

      inline Real realFromArray( V8Context& context, v8::Local<v8::Array> arrayValue, uint32_t index )
      {
        return static_cast<Real>(
          arrayValue->Get( context, index ).ToLocalChecked()->NumberValue( context ).FromMaybe( 0.0 ) );
      }

      inline Real extractNumberComponent( Isolate* isolate, v8::MaybeLocal<v8::Object>& obj, string_view name, Real defval )
      {
        if ( obj.IsEmpty() )
          return defval;
        auto val = obj.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( val.IsEmpty() || !val.ToLocalChecked()->IsNumber() )
          return defval;
        return static_cast<Real>( val.ToLocalChecked()->NumberValue( isolate->GetCurrentContext() ).FromMaybe( static_cast<double>( defval ) ) );
      }

      inline utf8String extractStringMember( Isolate* isolate, const utf8String& func, v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name )
      {
        const utf8String emptyString;
        if ( maybeObject.IsEmpty() )
        {
          util::throwException( isolate, ( "Syntax error: " + func + ": passed object is empty" ).c_str() );
          return emptyString;
        }
        auto object = maybeObject.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( object.IsEmpty() || !object.ToLocalChecked()->IsString() )
        {
          util::throwException( isolate, ( func + ": passed object has no string member \"" + name + "\"" ).c_str() );
          return emptyString;
        }
        v8::String::Utf8Value value( isolate, object.ToLocalChecked() );
        return ( *value ? *value : emptyString );
      }

      inline unicodeString extractStringMemberUnicode( Isolate* isolate, const utf8String& func,
        v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name, bool shouldThrow = true )
      {
        const unicodeString emptyString;
        if ( maybeObject.IsEmpty() )
        {
          if ( shouldThrow )
            util::throwException( isolate, ( "Syntax error: " + func + ": passed object is empty" ).c_str() );
          return emptyString;
        }
        auto object = maybeObject.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( object.IsEmpty() || !object.ToLocalChecked()->IsString() )
        {
          if ( shouldThrow )
            util::throwException( isolate, ( func + ": passed object has no string member \"" + name + "\"" ).c_str() );
          return emptyString;
        }
        v8::String::Value value( isolate, object.ToLocalChecked() );
        return ( *value ? unicodeString( *value ) : emptyString );
      }

      inline V8Function extractFunctionMember( Isolate* isolate, const utf8String& func, v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name, bool shouldThrow = true )
      {
        const V8Function emptyFunc;
        if ( maybeObject.IsEmpty() )
        {
          if ( shouldThrow )
            util::throwException( isolate, ( "Syntax error: " + func + ": passed object is empty" ).c_str() );
          return emptyFunc;
        }
        auto object = maybeObject.ToLocalChecked()->Get( isolate->GetCurrentContext(), util::allocStringConserve( name, isolate ) );
        if ( object.IsEmpty() || !object.ToLocalChecked()->IsFunction() )
        {
          if ( shouldThrow )
            util::throwException( isolate, ( func + ": passed object has no function member \"" + name + "\"" ).c_str() );
          return emptyFunc;
        }
        return V8Function::New( isolate, V8Function::Cast( object.ToLocalChecked() ) );
      }

    }

  }

}

#endif