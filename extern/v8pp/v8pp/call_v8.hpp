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

#include <v8.h>

#include "v8pp/config.hpp"
#include "v8pp/convert.hpp"

namespace v8pp {

  /// Call a V8 function, converting C++ arguments to Value arguments
  /// @param isolate V8 isolate instance
  /// @param func  V8 function to call
  /// @param recv V8 object used as `this` in the function
  /// @param args...  C++ arguments to convert to JS arguments using to_v8
  template <typename... TArgs>
  Local<Value> call_v8( Isolate* isolate, Local<Function> func, Local<Value> recv, TArgs&&... args )
  {
    EscapableHandleScope scope( isolate );

    int const arg_count = sizeof...( TArgs );
    // +1 to allocate array for arg_count == 0
    Local<Value> v8_args[arg_count + 1] =
    {
      to_v8( isolate, std::forward<TArgs>( args ) )... };

    Local<Value> result;
    bool const is_empty_result = func->Call( isolate->GetCurrentContext(), recv, arg_count, v8_args ).ToLocal( &result );
    (void)is_empty_result;

    return scope.Escape( result );
  }

}