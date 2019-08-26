#pragma once

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

#include <cstring>
#include <v8.h>

#include "v8pp/config.hpp"
#include "v8pp/convert.hpp"

namespace v8pp {

  /// Get optional value from V8 object by name.
  /// Dot symbols in option name delimits subobjects name.
  /// return false if the value doesn't exist in the options object
  template <typename T>
  bool get_option( Isolate* isolate, Local<Object> options, char const* name, T& value )
  {
    char const* dot = strchr( name, '.' );
    if ( dot )
    {
      utf8String const subname( name, dot );
      Local<Object> suboptions;
      return get_option( isolate, options, subname.c_str(), suboptions ) && get_option( isolate, suboptions, dot + 1, value );
    }
    Local<Value> val;
    if ( !options->Get( isolate->GetCurrentContext(), v8pp::to_v8( isolate, name ) ).ToLocal( &val ) || val->IsUndefined() )
    {
      return false;
    }
    value = from_v8<T>( isolate, val );
    return true;
  }

  /// Set named value in V8 object
  /// Dot symbols in option name delimits subobjects name.
  /// return false if the value doesn't exists in the options subobject
  template <typename T>
  bool set_option( Isolate* isolate, Local<Object> options, char const* name, T const& value )
  {
    char const* dot = strchr( name, '.' );
    if ( dot )
    {
      utf8String const subname( name, dot );
      HandleScope scope( isolate );
      Local<Object> suboptions;
      return get_option( isolate, options, subname.c_str(), suboptions ) && set_option( isolate, suboptions, dot + 1, value );
    }
    return options->Set( isolate->GetCurrentContext(), v8pp::to_v8( isolate, name ), to_v8( isolate, value ) ).FromJust();
  }

  /// Set named constant in V8 object
  /// Subobject names are not supported
  template <typename T>
  void set_const( Isolate* isolate, Local<Object> options, char const* name, T const& value )
  {
    options->DefineOwnProperty( isolate->GetCurrentContext(), v8pp::to_v8( isolate, name ), to_v8( isolate, value ), PropertyAttribute( v8::ReadOnly | v8::DontDelete ) ).FromJust();
  }

}