#include "stdafx.h"

// NOTE: This is NOT the original v8pp source!
// Some modifications have been made to fit the nekoengine project.

//
// Copyright (c) 2013-2016 Pavel Medvedev. All rights reserved.
//
// This file is part of v8pp (https://github.com/pmed/v8pp) project.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "v8pp/config.hpp"
#include "v8pp/json.hpp"

namespace v8pp {

  utf8String json_str( Isolate* isolate, Local<Value> value )
  {
    if ( value.IsEmpty() )
    {
      return utf8String();
    }

    HandleScope scope( isolate );

    Local<Context> context = isolate->GetCurrentContext();
    Local<String> result = v8::JSON::Stringify( context, value ).ToLocalChecked();
    String::Utf8Value const str( isolate, result );

    return utf8String( *str, str.length() );
  }

  Local<Value> json_parse( Isolate* isolate, utf8String const& str )
  {
    if ( str.empty() )
    {
      return Local<Value>();
    }

    EscapableHandleScope scope( isolate );

    Local<Context> context = isolate->GetCurrentContext();
    Local<String> value = String::NewFromUtf8( isolate, str.data(),
      v8::NewStringType::kNormal, static_cast<int>( str.size() ) ).ToLocalChecked();

    v8::TryCatch try_catch( isolate );
    Local<Value> result;
    bool const is_empty_result = v8::JSON::Parse( context, value ).ToLocal( &result );
    (void)is_empty_result;
    if ( try_catch.HasCaught() )
    {
      result = try_catch.Exception();
    }
    return scope.Escape( result );
  }

  Local<Object> json_object( Isolate* isolate, Local<Object> object, bool with_functions )
  {
    EscapableHandleScope scope( isolate );

    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> result = Object::New( isolate );
    Local<Array> prop_names = object->GetPropertyNames( context ).ToLocalChecked();
    for ( uint32_t i = 0, count = prop_names->Length(); i < count; ++i )
    {
      Local<Value> name, value;
      if ( prop_names->Get( context, i ).ToLocal( &name )
        && object->Get( context, name ).ToLocal( &value ) )
      {
        if ( value->IsFunction() )
        {
          if ( !with_functions ) continue;
          value = value.As<Function>()->ToString( context ).FromMaybe( String::Empty( isolate ) );
        } else if ( value->IsObject() || value->IsArray() )
        {
          value = v8::JSON::Stringify( context, value ).FromMaybe( String::Empty( isolate ) );
        }
        result->Set( context, name, value );
      }
    }
    return scope.Escape( result );
  }

}