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
#include <map>
#include <v8.h>

#include "v8pp/config.hpp"
#include "v8pp/convert.hpp"

namespace v8pp {

  class module;

  template <typename T, typename Traits>
  class class_;

  /// V8 isolate and context wrapper
  class context {
  public:
    /// Create context with optional existing v8::Isolate
    /// and v8::ArrayBuffer::Allocator,
    //  and add default global methods (`require()`, `run()`)
    explicit context( Isolate* isolate = nullptr, ArrayBuffer::Allocator* allocator = nullptr, bool add_default_global_methods = true );
    ~context();

    /// V8 isolate associated with this context
    Isolate* isolate() { return isolate_; }

    /// Library search path
    utf8String const& lib_path() const { return lib_path_; }

    /// Set new library search path
    void set_lib_path( utf8String const& lib_path ) { lib_path_ = lib_path; }

    /// Run script file, returns script result
    /// or empty handle on failure, use v8::TryCatch around it to find out why.
    /// Must be invoked in a v8::HandleScope
    Local<Value> run_file( utf8String const& filename );

    /// The same as run_file but uses string as the script source
    Local<Value> run_script( utf8String const& source, utf8String const& filename = "" );

    /// Set a V8 value in the context global object with specified name
    context& set( char const* name, Local<Value> value );

    /// Set module to the context global object
    context& set( char const* name, module& m );

    /// Set class to the context global object
    template <typename T, typename Traits>
    context& set( char const* name, class_<T, Traits>& cl )
    {
      HandleScope scope( isolate_ );
      cl.class_function_template()->SetClassName( v8pp::to_v8( isolate_, name ) );
      return set( name, cl.js_function_template()->GetFunction( isolate_->GetCurrentContext() ).ToLocalChecked() );
    }

  private:
    bool own_isolate_;
    Isolate* isolate_;
    Global<Context> impl_;

    struct dynamic_module;
    using dynamic_modules = std::map<utf8String, dynamic_module>;

    static void load_module( FunctionCallbackInfo<Value> const& args );
    static void run_file( FunctionCallbackInfo<Value> const& args );

    dynamic_modules modules_;
    utf8String lib_path_;
  };

}