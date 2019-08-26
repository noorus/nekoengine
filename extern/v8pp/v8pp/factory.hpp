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

#include <memory>
#include <v8.h>

#include "v8pp/config.hpp"

namespace v8pp {

  // Factory that creates new C++ objects of type T
  template <typename T, typename Traits>
  struct factory {
    using object_pointer_type = typename Traits::template object_pointer_type<T>;

    template <typename... Args>
    static object_pointer_type create( Isolate* isolate, Args... args )
    {
      object_pointer_type object = Traits::template create<T>( std::forward<Args>( args )... );
      isolate->AdjustAmountOfExternalAllocatedMemory(
        static_cast<int64_t>( Traits::object_size( object ) ) );
      return object;
    }

    static void destroy( Isolate* isolate, object_pointer_type const& object )
    {
      isolate->AdjustAmountOfExternalAllocatedMemory(
        -static_cast<int64_t>( Traits::object_size( object ) ) );
      Traits::destroy( object );
    }
  };

}