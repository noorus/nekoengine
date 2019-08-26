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

  Local<Value> throw_ex( Isolate* isolate, char const* str );

  Local<Value> throw_ex( Isolate* isolate, char const* str, Local<Value> ( *exception_ctor )( Local<String> ) );

  inline Local<Value> throw_ex( Isolate* isolate, utf8String const& str )
  {
    return throw_ex( isolate, str.c_str() );
  }

  inline Local<Value> throw_ex( Isolate* isolate, utf8String const& str, Local<Value> ( *exception_ctor )( Local<String> ) )
  {
    return throw_ex( isolate, str.c_str(), exception_ctor );
  }

}