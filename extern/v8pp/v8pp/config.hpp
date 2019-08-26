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

#if V8PP_HEADER_ONLY
# define V8PP_IMPL inline
#else
# define V8PP_IMPL
#endif