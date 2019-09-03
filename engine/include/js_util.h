#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "neko_exception.h"

#include <v8.h>

namespace neko {

  namespace js {

    namespace util {

#     define JS_TEMPLATE_ACCESSOR(tpl,x,y,z) tpl->PrototypeTemplate()->SetAccessor( \
        util::allocString( x ), y, z )

      //! Create a local JavaScript String instance from given source string.
      inline v8::Local<v8::String> allocString( const utf8String& str, v8::Isolate* isolate = nullptr )
      {
        v8::MaybeLocal<v8::String> ret = v8::String::NewFromUtf8(
          isolate ? isolate : v8::Isolate::GetCurrent(), str.c_str(), v8::NewStringType::kInternalized );
        if ( ret.IsEmpty() )
          NEKO_EXCEPT( "V8 String allocation failed" );
        return ret.ToLocalChecked();
      }

      //! Create a local JavaScript String instance from given source string.
      inline v8::Local<v8::String> allocString( const wstring& str, v8::Isolate* isolate = nullptr )
      {
        v8::MaybeLocal<v8::String> ret = v8::String::NewFromTwoByte(
          isolate ? isolate : v8::Isolate::GetCurrent(), (uint16_t*)str.c_str(), v8::NewStringType::kInternalized );
        if ( ret.IsEmpty() )
          NEKO_EXCEPT( "V8 String allocation failed" );
        return ret.ToLocalChecked();
      }

      //! Create a local V8 String that will likely be duplicated (class names etc.)
      inline v8::Local<v8::String> allocStringConserve( string_view str, v8::Isolate* isolate = nullptr )
      {
        auto ret = v8::String::NewFromUtf8( isolate ? isolate : v8::Isolate::GetCurrent(), str.data(), v8::NewStringType::kInternalized );
        if ( ret.IsEmpty() )
          NEKO_EXCEPT( "V8 String allocation failed" );
        return ret.ToLocalChecked();
      }

      inline void throwException( v8::Isolate* isolate, const wchar_t* message, ... )
      {
        wchar_t buffer[1024];

        va_list va_alist;
        va_start( va_alist, message );
        _vsnwprintf_s( buffer, 1024, message, va_alist );
        va_end( va_alist );

        isolate->ThrowException( allocString( buffer, isolate ) );
      }

    }

  }

}

#endif // !NEKO_NO_SCRIPTING