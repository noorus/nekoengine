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

#include "neko_types.h"

// v8::Isolate data slot number, used in v8pp for shared data
#define V8PP_ISOLATE_DATA_SLOT 0

// v8pp plugin initialization procedure name
#define V8PP_PLUGIN_INIT_PROC_NAME v8pp_module_init

// v8pp plugin filename suffix
#define V8PP_PLUGIN_SUFFIX ".dll"

// not header only.
#define V8PP_HEADER_ONLY 0

#if defined( _MSC_VER )
# define V8PP_EXPORT __declspec( dllexport )
# define V8PP_IMPORT __declspec( dllimport )
#elif __GNUC__ >= 4
# define V8PP_EXPORT __attribute__( ( __visibility__( "default" ) ) )
# define V8PP_IMPORT V8PP_EXPORT
#else
# define V8PP_EXPORT
# define V8PP_IMPORT
#endif

#define V8PP_PLUGIN_INIT( isolate ) extern "C" V8PP_EXPORT \
  v8::Local<v8::Value>                                     \
    V8PP_PLUGIN_INIT_PROC_NAME( isolate )

namespace v8pp {

  using neko::vector;
  using neko::unique_ptr;
  using neko::shared_ptr;
  using neko::move;
  using neko::utf8String;

#ifndef NEKO_NO_ICU
  using neko::unicodeString;
  using neko::unicodePiece;
#endif

  using v8::HandleScope;
  using v8::EscapableHandleScope;
  using v8::Isolate;
  using v8::Context;
  using v8::Local;
  using v8::External;
  using v8::Global;
  using v8::PersistentBase;
  using v8::Value;
  using v8::ObjectTemplate;
  using v8::Object;
  using v8::PropertyAttribute;
  using v8::Function;
  using v8::FunctionTemplate;
  using v8::FunctionCallbackInfo;
  using v8::PropertyCallbackInfo;
  using v8::AccessorGetterCallback;
  using v8::AccessorSetterCallback;
  using v8::WeakCallbackInfo;
  using v8::WeakCallbackType;
  using v8::Number;
  using v8::Integer;
  using v8::ArrayBuffer;
  using v8::Boolean;
  using v8::Array;
  using v8::String;
  using v8::Data;

}