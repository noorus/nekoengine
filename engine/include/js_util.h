#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "neko_exception.h"

#include <v8.h>

namespace neko {

  namespace js {

    namespace util {

      //! Create a local JavaScript String instance from given source string.
      inline v8::Local<v8::String> allocString( const utf8String& str, v8::Isolate* isolate = nullptr )
      {
        v8::MaybeLocal<v8::String> ret = v8::String::NewFromUtf8(
          isolate ? isolate : v8::Isolate::GetCurrent(), str.c_str(), v8::NewStringType::kInternalized );
        if ( ret.IsEmpty() )
          NEKO_EXCEPT( "String memory allocation failed" );
        return ret.ToLocalChecked();
      }

    }

  }

}

#endif // !NEKO_NO_SCRIPTING