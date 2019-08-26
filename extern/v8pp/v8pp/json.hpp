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

#include <string>
#include <v8.h>

#include "v8pp/config.hpp"

namespace v8pp {

  /// Stringify V8 value to JSON
  /// return empty string for empty value
  utf8String json_str( Isolate* isolate, Local<Value> value );

  /// Parse JSON string into V8 value
  /// return empty value for empty string
  /// return Error value on parse error
  Local<Value> json_parse( Isolate* isolate, utf8String const& str );

  /// Convert wrapped C++ object to JavaScript object with properties
  /// and optionally functions set from the C++ object
  Local<Object> json_object( Isolate* isolate, Local<Object> object, bool with_functions = false );

}