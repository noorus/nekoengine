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

#include <algorithm>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "v8pp/config.hpp"
#include "v8pp/factory.hpp"
#include "v8pp/function.hpp"
#include "v8pp/property.hpp"
#include "v8pp/ptr_traits.hpp"

namespace v8pp {

  namespace detail {

    struct ClassInfo
    {
      TypeInfo const type;
      TypeInfo const traits;

      ClassInfo( TypeInfo const& type, TypeInfo const& traits );

      virtual ~ClassInfo() = default; // make virtual to delete derived object_registry

      utf8String class_name() const;
    };

    template <typename Traits>
    class ObjectRegistry final: public ClassInfo {
    public:
      using TPointer = typename Traits::pointer_type;
      using TConstPointer = typename Traits::const_pointer_type;
      using TObjectID = typename Traits::object_id;

      using ctor_function = std::function<TPointer( FunctionCallbackInfo<Value> const& args )>;
      using dtor_function = std::function<void( Isolate*, TPointer const& )>;
      using cast_function = TPointer( *)( TPointer const& );

      ObjectRegistry( Isolate* isolate, TypeInfo const& type, dtor_function&& dtor );

      ObjectRegistry( ObjectRegistry const& ) = delete;
      ObjectRegistry( ObjectRegistry&& src ) = default;

      ObjectRegistry& operator=( ObjectRegistry const& ) = delete;
      ObjectRegistry& operator=( ObjectRegistry&& ) = delete;

      ~ObjectRegistry();

      inline Isolate* isolate() { return isolate_; }

      Local<FunctionTemplate> class_function_template()
      {
        return to_local( isolate_, func_ );
      }

      Local<FunctionTemplate> js_function_template()
      {
        return to_local( isolate_, js_func_ );
      }

      void set_auto_wrap_objects( bool auto_wrap ) { auto_wrap_objects_ = auto_wrap; }
      bool auto_wrap_objects() const { return auto_wrap_objects_; }

      void set_ctor( ctor_function&& ctor ) { ctor_ = move( ctor ); }

      void add_base( ObjectRegistry& info, cast_function cast );

      bool cast( TPointer& ptr, TypeInfo const& type ) const;

      void remove_object( TObjectID const& obj );
      void remove_objects();

      TPointer find_object( TObjectID id, TypeInfo const& type ) const;
      Local<Object> find_v8_object( TPointer const& ptr ) const;

      Local<Object> wrap( TPointer const& object, bool call_dtor );
      Local<Object> wrap( FunctionCallbackInfo<Value> const& args );
      TPointer unwrap( Local<Value> value );

    private:
      struct WrappedObject
      {
        Global<Object> pobj;
        bool call_dtor;
      };

      void reset_object( TPointer const& object, WrappedObject& wrapped );

      struct BaseClassInfo
      {
        ObjectRegistry& info;
        cast_function cast;
        BaseClassInfo( ObjectRegistry& info, cast_function cast )
          : info( info ), cast( cast )
        {
        }
      };

      std::vector<BaseClassInfo> bases_;
      std::vector<ObjectRegistry*> derivatives_;
      std::unordered_map<TPointer, WrappedObject> objects_;

      Isolate* isolate_;
      Global<FunctionTemplate> func_;
      Global<FunctionTemplate> js_func_;

      ctor_function ctor_;
      dtor_function dtor_;
      bool auto_wrap_objects_;
    };

    class Classes {
    public:
      template <typename Traits>
      static ObjectRegistry<Traits>& add( Isolate* isolate, TypeInfo const& type, typename ObjectRegistry<Traits>::dtor_function&& dtor );

      template <typename Traits>
      static void remove( Isolate* isolate, TypeInfo const& type );

      template <typename Traits>
      static ObjectRegistry<Traits>& find( Isolate* isolate, TypeInfo const& type );

      static void remove_all( Isolate* isolate );

    private:
      using ClassInfoVector = vector<unique_ptr<ClassInfo>>;
      ClassInfoVector classes_;

      ClassInfoVector::iterator find( TypeInfo const& type );

      enum class operation {
        get,
        add,
        remove
      };
      static Classes* instance( operation op, Isolate* isolate );
    };

  } // namespace detail

  /// Interface to access C++ classes bound to V8
  template <typename T, typename Traits = raw_ptr_traits>
  class class_ {
    static_assert( is_wrapped_class<T>::value, "T must be a user-defined class" );

    using TRegistry = detail::ObjectRegistry<Traits>;
    TRegistry& class_info_;

    using object_id = typename TRegistry::TObjectID;
    using TPointer = typename TRegistry::TPointer;
    using TConstPointer = typename TRegistry::TConstPointer;

  public:
    using object_pointer_type = typename Traits::template object_pointer_type<T>;
    using object_const_pointer_type = typename Traits::template object_const_pointer_type<T>;

    template <typename... Args>
    struct factory_create {
      static object_pointer_type call( FunctionCallbackInfo<Value> const& args )
      {
        using ctor_function = object_pointer_type( *)( Isolate * isolate, Args... );
        return detail::call_from_v8<Traits, ctor_function>( &factory<T, Traits>::create, args );
      }
    };

    using ctor_function = std::function<object_pointer_type( FunctionCallbackInfo<Value> const& args )>;
    using dtor_function = std::function<void( Isolate* isolate, object_pointer_type const& obj )>;

  public:
    explicit class_( Isolate* isolate, dtor_function destroy = &factory<T, Traits>::destroy )
      : class_info_( detail::Classes::add<Traits>( isolate, detail::type_id<T>(), [destroy = move( destroy )]( Isolate* isolate, TPointer const& obj ) {
      destroy( isolate, Traits::template static_pointer_cast<T>( obj ) );
    } ) )
    {
    }

    /// Set class constructor signature
    template <typename... Args, typename Create = factory_create<Args...>>
    class_& ctor( ctor_function create = &Create::call )
    {
      class_info_.set_ctor( [create = move( create )]( FunctionCallbackInfo<Value> const& args ) {
        return create( args );
      } );
      return *this;
    }

    /// Inhert from C++ class U
    template <typename U>
    class_& inherit()
    {
      using namespace detail;
      static_assert( std::is_base_of<U, T>::value, "Class U should be base for class T" );
      //TODO: std::is_convertible<T*, U*> and check for duplicates in hierarchy?
      TRegistry& base = Classes::find<Traits>( isolate(), type_id<U>() );
      class_info_.add_base( base, []( TPointer const& ptr ) -> pointer_type {
        return TPointer( Traits::template static_pointer_cast<U>(
          Traits::template static_pointer_cast<T>( ptr ) ) );
      } );
      class_info_.js_function_template()->Inherit( base.class_function_template() );
      return *this;
    }

    /// Enable new C++ objects auto-wrapping
    class_& auto_wrap_objects( bool auto_wrap = true )
    {
      class_info_.set_auto_wrap_objects( auto_wrap );
      return *this;
    }

    /// Set C++ class member function
    template <typename Method>
    typename std::enable_if<
      std::is_member_function_pointer<Method>::value, class_&>::type
      set( char const* name, Method mem_func, PropertyAttribute attr = v8::None )
    {
      using mem_func_type =
        typename detail::function_traits<Method>::template TPointer<T>;
      mem_func_type mf( mem_func );
      class_info_.class_function_template()->PrototypeTemplate()->Set(
        v8pp::to_v8( isolate(), name ), FunctionTemplate::New( isolate(), &detail::forward_function<Traits, mem_func_type>, detail::set_external_data( isolate(), std::forward<mem_func_type>( mf ) ) ), attr );
      return *this;
    }

    /// Set static class function
    template <typename Function, typename Func = typename std::decay<Function>::type>
    typename std::enable_if<detail::is_callable<Func>::value, class_&>::type
      set( char const* name, Function&& func, PropertyAttribute attr = v8::None )
    {
      Local<Data> wrapped_fun =
        wrap_function_template( isolate(), std::forward<Func>( func ) );
      class_info_.class_function_template()
        ->PrototypeTemplate()
        ->Set( v8pp::to_v8( isolate(), name ), wrapped_fun, attr );
      class_info_.js_function_template()->Set( v8pp::to_v8( isolate(), name ), wrapped_fun, attr );
      return *this;
    }

    /// Set class member data
    template <typename Attribute>
    typename std::enable_if<
      std::is_member_object_pointer<Attribute>::value, class_&>::type
      set( char const* name, Attribute attribute, bool readonly = false )
    {
      HandleScope scope( isolate() );

      using attribute_type = typename detail::function_traits<Attribute>::template TPointer<T>;
      attribute_type attr( attribute );
      AccessorGetterCallback getter = &member_get<attribute_type>;
      AccessorSetterCallback setter = &member_set<attribute_type>;
      if ( readonly )
      {
        setter = nullptr;
      }

      class_info_.class_function_template()->PrototypeTemplate()->SetAccessor( v8pp::to_v8( isolate(), name ), getter, setter, detail::set_external_data( isolate(), std::forward<attribute_type>( attr ) ), v8::DEFAULT, PropertyAttribute( v8::DontDelete | ( setter ? 0 : v8::ReadOnly ) ) );
      return *this;
    }

    /// Set read/write class property with getter and setter
    template <typename GetMethod, typename SetMethod>
    typename std::enable_if<std::is_member_function_pointer<GetMethod>::value && std::is_member_function_pointer<SetMethod>::value, class_&>::type
      set( char const* name, property_<GetMethod, SetMethod>&& property )
    {
      HandleScope scope( isolate() );

      using property_type = property_<
        typename detail::function_traits<GetMethod>::template TPointer<T>,
        typename detail::function_traits<SetMethod>::template TPointer<T>>;
      property_type prop( property );
      AccessorGetterCallback getter = property_type::template get<Traits>;
      AccessorSetterCallback setter = property_type::template set<Traits>;
      if ( prop.is_readonly )
      {
        setter = nullptr;
      }

      class_info_.class_function_template()->PrototypeTemplate()->SetAccessor( v8pp::to_v8( isolate(), name ), getter, setter, detail::set_external_data( isolate(), std::forward<property_type>( prop ) ), v8::DEFAULT, PropertyAttribute( v8::DontDelete | ( setter ? 0 : v8::ReadOnly ) ) );
      return *this;
    }

    /// Set value as a read-only property
    template <typename Value>
    class_& set_const( char const* name, Value const& value )
    {
      HandleScope scope( isolate() );

      class_info_.class_function_template()->PrototypeTemplate()->Set( v8pp::to_v8( isolate(), name ), to_v8( isolate(), value ), PropertyAttribute( v8::ReadOnly | v8::DontDelete ) );
      return *this;
    }

    /// Set a static value
    template <typename Value>
    class_& set_static( char const* name, Value const& value, bool readonly = false )
    {
      HandleScope scope( isolate() );

      class_info_.js_function_template()->GetFunction( isolate()->GetCurrentContext() ).ToLocalChecked()->DefineOwnProperty( isolate()->GetCurrentContext(), v8pp::to_v8( isolate(), name ), to_v8( isolate(), value ), PropertyAttribute( v8::DontDelete | ( readonly ? v8::ReadOnly : 0 ) ) ).FromJust();
      return *this;
    }

    /// Isolate where the class bindings belongs
    Isolate* isolate() { return class_info_.isolate(); }

    Local<FunctionTemplate> class_function_template()
    {
      return class_info_.class_function_template();
    }

    Local<FunctionTemplate> js_function_template()
    {
      return class_info_.js_function_template();
    }

    /// Create JavaScript object which references externally created C++ class.
    /// It will not take ownership of the C++ pointer.
    static Local<Object> reference_external( Isolate* isolate, object_pointer_type const& ext )
    {
      using namespace detail;
      return Classes::find<Traits>( isolate, type_id<T>() ).wrap( ext, false );
    }

    /// Remove external reference from JavaScript
    static void unreference_external( Isolate* isolate, object_pointer_type const& ext )
    {
      using namespace detail;
      return Classes::find<Traits>( isolate, type_id<T>() ).remove_object( Traits::pointer_id( ext ) );
    }

    /// As reference_external but delete memory for C++ object
    /// when JavaScript object is deleted. You must use `factory<T>::create()`
    /// to allocate `ext`
    static Local<Object> import_external( Isolate* isolate, object_pointer_type const& ext )
    {
      using namespace detail;
      return Classes::find<Traits>( isolate, type_id<T>() ).wrap( ext, true );
    }

    /// Get wrapped object from V8 value, may return nullptr on fail.
    static object_pointer_type unwrap( Isolate* isolate, Local<Value> value )
    {
      using namespace detail;
      return Traits::template static_pointer_cast<T>(
        Classes::find<Traits>( isolate, type_id<T>() ).unwrap( value ) );
    }

    /// Create a wrapped C++ object and import it into JavaScript
    template <typename... Args>
    static Local<Object> create_object( Isolate* isolate, Args&&... args )
    {
      return import_external( isolate, factory<T, Traits>::create( isolate, std::forward<Args>( args )... ) );
    }

    /// Find V8 object handle for a wrapped C++ object, may return empty handle on fail.
    static Local<Object> find_object( Isolate* isolate, object_const_pointer_type const& obj )
    {
      using namespace detail;
      return Classes::find<Traits>( isolate, type_id<T>() )
        .find_v8_object( Traits::const_pointer_cast( obj ) );
    }

    /// Find V8 object handle for a wrapped C++ object, may return empty handle on fail
    /// or wrap a copy of the obj if class_.auto_wrap_objects()
    static Local<Object> find_object( Isolate* isolate, T const& obj )
    {
      using namespace detail;
      detail::ObjectRegistry<Traits>& class_info = Classes::find<Traits>( isolate, type_id<T>() );
      Local<Object> wrapped_object = class_info.find_v8_object( Traits::key( const_cast<T*>( &obj ) ) );
      if ( wrapped_object.IsEmpty() && class_info.auto_wrap_objects() )
      {
        object_pointer_type clone = Traits::clone( obj );
        if ( clone )
        {
          wrapped_object = class_info.wrap( clone, true );
        }
      }
      return wrapped_object;
    }

    /// Destroy wrapped C++ object
    static void destroy_object( Isolate* isolate, object_pointer_type const& obj )
    {
      using namespace detail;
      Classes::find<Traits>( isolate, type_id<T>() ).remove_object( Traits::pointer_id( obj ) );
    }

    /// Destroy all wrapped C++ objects of this class
    static void destroy_objects( Isolate* isolate )
    {
      using namespace detail;
      Classes::find<Traits>( isolate, type_id<T>() ).remove_objects();
    }

    /// Destroy all wrapped C++ objects and this binding class_
    static void destroy( Isolate* isolate )
    {
      using namespace detail;
      Classes::remove<Traits>( isolate, type_id<T>() );
    }

  private:
    template <typename Attribute>
    static void member_get( Local<String>, PropertyCallbackInfo<Value> const& info )
    {
      Isolate* isolate = info.GetIsolate();

      try
      {
        auto self = unwrap( isolate, info.This() );
        Attribute attr = detail::get_external_data<Attribute>( info.Data() );
        info.GetReturnValue().Set( to_v8( isolate, ( *self ).*attr ) );
      } catch ( std::exception const& ex )
      {
        info.GetReturnValue().Set( throw_ex( isolate, ex.what() ) );
      }
    }

    template <typename Attribute>
    static void member_set( Local<String>, Local<Value> value, PropertyCallbackInfo<void> const& info )
    {
      Isolate* isolate = info.GetIsolate();

      try
      {
        auto self = unwrap( isolate, info.This() );
        Attribute ptr = detail::get_external_data<Attribute>( info.Data() );
        using attr_type = typename detail::function_traits<Attribute>::return_type;
        ( *self ).*ptr = v8pp::from_v8<attr_type>( isolate, value );
      } catch ( std::exception const& ex )
      {
        info.GetReturnValue().Set( throw_ex( isolate, ex.what() ) );
      }
    }
  };

  /// Interface to access C++ classes bound to V8
  /// Objects are stored in std::shared_ptr
  template <typename T>
  using shared_class = class_<T, shared_ptr_traits>;

  inline void cleanup( Isolate* isolate )
  {
    detail::Classes::remove_all( isolate );
  }

}