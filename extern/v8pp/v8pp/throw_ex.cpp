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
#include "v8pp/throw_ex.hpp"

namespace v8pp {

  Local<Value> throw_ex( Isolate* isolate, char const* str )
  {
    return isolate->ThrowException( String::NewFromUtf8( isolate, str ).ToLocalChecked() );
  }

  Local<Value> throw_ex( Isolate* isolate, char const* str, Local<Value>( *exception_ctor )( Local<String> ) )
  {
    return isolate->ThrowException( exception_ctor( String::NewFromUtf8( isolate, str ).ToLocalChecked() ) );
  }

}