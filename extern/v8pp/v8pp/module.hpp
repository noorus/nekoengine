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
#include "v8pp/function.hpp"
#include "v8pp/property.hpp"

namespace v8pp {

  template <typename T, typename Traits>
  class class_;

  /// Module (similar to v8::ObjectTemplate)
  class module {
  public:
    /// Create new module in the specified V8 isolate
    explicit module( Isolate* isolate )
      : isolate_( isolate ), obj_( ObjectTemplate::New( isolate ) )
    {
    }

    /// Create new module in the specified V8 isolate for existing ObjectTemplate
    explicit module( Isolate* isolate, Local<ObjectTemplate> obj )
      : isolate_( isolate ), obj_( obj )
    {
    }

    /// v8::Isolate where the module belongs
    Isolate* isolate() { return isolate_; }

    /// Set a V8 value in the module with specified name
    template <typename Data>
    module& set( char const* name, Local<Data> value )
    {
      obj_->Set( v8pp::to_v8( isolate_, name ), value );
      return *this;
    }

    /// Set another module in the module with specified name
    module& set( char const* name, module& m )
    {
      return set( name, m.obj_ );
    }

    /// Set wrapped C++ class in the module with specified name
    template <typename T, typename Traits>
    module& set( char const* name, class_<T, Traits>& cl )
    {
      HandleScope scope( isolate_ );

      cl.class_function_template()->SetClassName( v8pp::to_v8( isolate_, name ) );
      return set( name, cl.js_function_template() );
    }

    /// Set a C++ function in the module with specified name
    template <typename Function, typename Fun = typename std::decay<Function>::type>
    typename std::enable_if<detail::is_callable<Fun>::value, module&>::type
      set( char const* name, Function&& func )
    {
      return set( name, wrap_function_template( isolate_, std::forward<Fun>( func ) ) );
    }

    /// Set a C++ variable in the module with specified name
    template <typename Variable>
    typename std::enable_if<!detail::is_callable<Variable>::value, module&>::type
      set( char const* name, Variable& var, bool readonly = false )
    {
      HandleScope scope( isolate_ );

      AccessorGetterCallback getter = &var_get<Variable>;
      AccessorSetterCallback setter = &var_set<Variable>;
      if ( readonly )
      {
        setter = nullptr;
      }

      obj_->SetAccessor( v8pp::to_v8( isolate_, name ), getter, setter, detail::set_external_data( isolate_, &var ), v8::DEFAULT, PropertyAttribute( v8::DontDelete | ( setter ? 0 : v8::ReadOnly ) ) );
      return *this;
    }

    /// Set v8pp::property in the module with specified name
    template <typename GetFunction, typename SetFunction>
    module& set( char const* name, property_<GetFunction, SetFunction>&& property )
    {
      using property_type = property_<GetFunction, SetFunction>;

      HandleScope scope( isolate_ );

      AccessorGetterCallback getter = property_type::get;
      AccessorSetterCallback setter = property_type::set;
      if ( property_type::is_readonly )
      {
        setter = nullptr;
      }

      obj_->SetAccessor( v8pp::to_v8( isolate_, name ), getter, setter, detail::set_external_data( isolate_, std::forward<property_type>( property ) ), v8::DEFAULT, PropertyAttribute( v8::DontDelete | ( setter ? 0 : v8::ReadOnly ) ) );
      return *this;
    }

    /// Set another module as a read-only property
    module& set_const( char const* name, module& m )
    {
      HandleScope scope( isolate_ );

      obj_->Set( v8pp::to_v8( isolate_, name ), m.obj_, PropertyAttribute( v8::ReadOnly | v8::DontDelete ) );
      return *this;
    }

    /// Set a value convertible to JavaScript as a read-only property
    template <typename Value>
    module& set_const( char const* name, Value const& value )
    {
      HandleScope scope( isolate_ );

      obj_->Set( v8pp::to_v8( isolate_, name ), to_v8( isolate_, value ), PropertyAttribute( v8::ReadOnly | v8::DontDelete ) );
      return *this;
    }

    /// Create a new module instance in V8
    Local<Object> new_instance()
    {
      return obj_->NewInstance( isolate_->GetCurrentContext() ).ToLocalChecked();
    }

  private:
    template <typename Variable>
    static void var_get( Local<String>, PropertyCallbackInfo<Value> const& info )
    {
      Isolate* isolate = info.GetIsolate();

      Variable* var = detail::get_external_data<Variable*>( info.Data() );
      info.GetReturnValue().Set( to_v8( isolate, *var ) );
    }

    template <typename Variable>
    static void var_set( Local<String>, Local<Value> value, PropertyCallbackInfo<void> const& info )
    {
      Isolate* isolate = info.GetIsolate();

      Variable* var = detail::get_external_data<Variable*>( info.Data() );
      *var = v8pp::from_v8<Variable>( isolate, value );
    }

    Isolate* isolate_;
    Local<ObjectTemplate> obj_;
  };

}