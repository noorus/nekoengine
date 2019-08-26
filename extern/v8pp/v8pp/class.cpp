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
#include "v8pp/class.hpp"

namespace v8pp {

  namespace detail {

    static utf8String pointer_str( void const* ptr )
    {
      utf8String buf( sizeof( void* ) * 2 + 3, 0 ); // +3 for 0x and \0 terminator
      int const len =
#if defined(_MSC_VER) && (_MSC_VER < 1900)
        sprintf_s( &buf[0], buf.size(), "%p", ptr );
#else
        snprintf( &buf[0], buf.size(), "%p", ptr );
#endif
      buf.resize( len < 0 ? 0 : len );
      return buf;
    }

    /////////////////////////////////////////////////////////////////////////////
    //
    // class_info
    //
    ClassInfo::ClassInfo( TypeInfo const& type, TypeInfo const& traits )
      : type( type )
      , traits( traits )
    {
    }

    utf8String ClassInfo::class_name() const
    {
      return "v8pp::class_<" + type.name() + ", " + traits.name() + ">";
    }

    /////////////////////////////////////////////////////////////////////////////
    //
    // object_registry
    //
    template<typename Traits>
    ObjectRegistry<Traits>::ObjectRegistry( Isolate* isolate, TypeInfo const& type, dtor_function&& dtor )
      : ClassInfo( type, type_id<Traits>() )
      , isolate_( isolate )
      , ctor_() // no wrapped class constructor available by default
      , dtor_( move( dtor ) )
      , auto_wrap_objects_( false )
    {
      HandleScope scope( isolate_ );

      Local<FunctionTemplate> func = FunctionTemplate::New( isolate_ );
      Local<FunctionTemplate> js_func = FunctionTemplate::New( isolate_,
        []( FunctionCallbackInfo<Value> const& args )
      {
        Isolate* isolate = args.GetIsolate();
        ObjectRegistry* this_ = get_external_data<ObjectRegistry*>( args.Data() );
        try
        {
          return args.GetReturnValue().Set( this_->wrap( args ) );
        } catch ( std::exception const& ex )
        {
          args.GetReturnValue().Set( throw_ex( isolate, ex.what() ) );
        }
      }, set_external_data( isolate, this ) );

      func_.Reset( isolate, func );
      js_func_.Reset( isolate, js_func );

      // each JavaScript instance has 3 internal fields:
      //  0 - pointer to a wrapped C++ object
      //  1 - pointer to this object_registry
      func->InstanceTemplate()->SetInternalFieldCount( 2 );
      func->Inherit( js_func );
    }

    template<typename Traits>
    ObjectRegistry<Traits>::~ObjectRegistry()
    {
      remove_objects();
    }

    template<typename Traits>
    void ObjectRegistry<Traits>::add_base( ObjectRegistry& info, cast_function cast )
    {
      auto it = std::find_if( bases_.begin(), bases_.end(),
        [&info]( BaseClassInfo const& base ) { return &base.info == &info; } );
      if ( it != bases_.end() )
      {
        //assert(false && "duplicated inheritance");
        throw std::runtime_error( class_name()
          + " is already inherited from " + info.class_name() );
      }
      bases_.emplace_back( info, cast );
      info.derivatives_.emplace_back( this );
    }

    template<typename Traits>
    bool ObjectRegistry<Traits>::cast( TPointer& ptr, TypeInfo const& type ) const
    {
      if ( this->type == type || !ptr )
      {
        return true;
      }

      // fast way - search a direct parent
      for ( BaseClassInfo const& base : bases_ )
      {
        if ( base.info.type == type )
        {
          ptr = base.cast( ptr );
          return true;
        }
      }

      // slower way - walk on hierarhy
      for ( BaseClassInfo const& base : bases_ )
      {
        TPointer p = base.cast( ptr );
        if ( base.info.cast( p, type ) )
        {
          ptr = p;
          return true;
        }
      }
      return false;
    }

    template<typename Traits>
    void ObjectRegistry<Traits>::remove_object( TObjectID const& obj )
    {
      auto it = objects_.find( Traits::key( obj ) );
      assert( it != objects_.end() && "no object" );
      if ( it != objects_.end() )
      {
        HandleScope scope( isolate_ );
        reset_object( it->first, it->second );
        objects_.erase( it );
      }
    }

    template<typename Traits>
    void ObjectRegistry<Traits>::remove_objects()
    {
      HandleScope scope( isolate_ );
      for ( auto& object_wrapped : objects_ )
      {
        reset_object( object_wrapped.first, object_wrapped.second );
      }
      objects_.clear();
    }

    template<typename Traits>
    typename ObjectRegistry<Traits>::TPointer
      ObjectRegistry<Traits>::find_object( TObjectID id, TypeInfo const& type ) const
    {
      auto it = objects_.find( Traits::key( id ) );
      if ( it != objects_.end() )
      {
        TPointer ptr = it->first;
        if ( cast( ptr, type ) )
        {
          return ptr;
        }
      }
      return nullptr;
    }

    template<typename Traits>
    Local<Object> ObjectRegistry<Traits>::find_v8_object( TPointer const& ptr ) const
    {
      auto it = objects_.find( ptr );
      if ( it != objects_.end() )
      {
        return to_local( isolate_, it->second.pobj );
      }

      Local<Object> result;
      for ( auto const info : derivatives_ )
      {
        result = info->find_v8_object( ptr );
        if ( !result.IsEmpty() ) break;
      }
      return result;
    }

    template<typename Traits>
    Local<Object> ObjectRegistry<Traits>::wrap( TPointer const& object, bool call_dtor )
    {
      auto it = objects_.find( object );
      if ( it != objects_.end() )
      {
        //assert(false && "duplicate object");
        throw std::runtime_error( class_name()
          + " duplicate object " + pointer_str( Traits::pointer_id( object ) ) );
      }

      EscapableHandleScope scope( isolate_ );

      Local<Context> context = isolate_->GetCurrentContext();
      Local<Object> obj = class_function_template()
        ->GetFunction( context ).ToLocalChecked()->NewInstance( context ).ToLocalChecked();

      obj->SetAlignedPointerInInternalField( 0, Traits::pointer_id( object ) );
      obj->SetAlignedPointerInInternalField( 1, this );

      Global<Object> pobj( isolate_, obj );
      pobj.SetWeak( this, []( WeakCallbackInfo<ObjectRegistry> const& data )
      {
        TObjectID object = data.GetInternalField( 0 );
        ObjectRegistry* this_ = static_cast<ObjectRegistry*>( data.GetInternalField( 1 ) );
        this_->remove_object( object );
      }, WeakCallbackType::kInternalFields );
      objects_.emplace( object, WrappedObject{ move( pobj ), call_dtor } );

      return scope.Escape( obj );
    }

    template<typename Traits>
    Local<Object> ObjectRegistry<Traits>::wrap( FunctionCallbackInfo<Value> const& args )
    {
      if ( !ctor_ )
      {
        //assert(false && "create not allowed");
        throw std::runtime_error( class_name() + " has no constructor" );
      }
      return wrap( ctor_( args ), true );
    }

    template<typename Traits>
    typename ObjectRegistry<Traits>::TPointer
      ObjectRegistry<Traits>::unwrap( Local<Value> value )
    {
      HandleScope scope( isolate_ );

      while ( value->IsObject() )
      {
        Local<Object> obj = value.As<Object>();
        if ( obj->InternalFieldCount() == 2 )
        {
          TObjectID id = obj->GetAlignedPointerFromInternalField( 0 );
          if ( id )
          {
            auto registry = static_cast<ObjectRegistry*>(
              obj->GetAlignedPointerFromInternalField( 1 ) );
            if ( registry )
            {
              TPointer ptr = registry->find_object( id, type );
              if ( ptr )
              {
                return ptr;
              }
            }
          }
        }
        value = obj->GetPrototype();
      }
      return nullptr;
    }

    template<typename Traits>
    void ObjectRegistry<Traits>::reset_object( TPointer const& object, WrappedObject& wrapped )
    {
      if ( wrapped.call_dtor )
      {
        dtor_( isolate_, object );
      }
      wrapped.pobj.Reset();
    }

    /////////////////////////////////////////////////////////////////////////////
    //
    // classes
    //
    template<typename Traits>
    ObjectRegistry<Traits>& Classes::add( Isolate* isolate, TypeInfo const& type,
      typename ObjectRegistry<Traits>::dtor_function&& dtor )
    {
      Classes* info = instance( operation::add, isolate );
      auto it = info->find( type );
      if ( it != info->classes_.end() )
      {
        //assert(false && "class already registred");
        throw std::runtime_error( ( *it )->class_name()
          + " is already exist in isolate " + pointer_str( isolate ) );
      }
      info->classes_.emplace_back( new ObjectRegistry<Traits>( isolate, type, move( dtor ) ) );
      return *static_cast<ObjectRegistry<Traits>*>( info->classes_.back().get() );
    }

    template<typename Traits>
    void Classes::remove( Isolate* isolate, TypeInfo const& type )
    {
      Classes* info = instance( operation::get, isolate );
      if ( info )
      {
        auto it = info->find( type );
        if ( it != info->classes_.end() )
        {
          TypeInfo const& traits = type_id<Traits>();
          if ( ( *it )->traits != traits )
          {
            throw std::runtime_error( ( *it )->class_name()
              + " is already registered in isolate "
              + pointer_str( isolate ) + " before of "
              + ClassInfo( type, traits ).class_name() );
          }
          info->classes_.erase( it );
          if ( info->classes_.empty() )
          {
            instance( operation::remove, isolate );
          }
        }
      }
    }

    template<typename Traits>
    ObjectRegistry<Traits>& Classes::find( Isolate* isolate, TypeInfo const& type )
    {
      Classes* info = instance( operation::get, isolate );
      TypeInfo const& traits = type_id<Traits>();
      if ( info )
      {
        auto it = info->find( type );
        if ( it != info->classes_.end() )
        {
          if ( ( *it )->traits != traits )
          {
            throw std::runtime_error( ( *it )->class_name()
              + " is already registered in isolate "
              + pointer_str( isolate ) + " before of "
              + ClassInfo( type, traits ).class_name() );
          }
          return *static_cast<ObjectRegistry<Traits>*>( it->get() );
        }
      }
      //assert(false && "class not registered");
      throw std::runtime_error( ClassInfo( type, traits ).class_name()
        + " is not registered in isolate " + pointer_str( isolate ) );
    }

    void Classes::remove_all( Isolate* isolate )
    {
      instance( operation::remove, isolate );
    }

    Classes::ClassInfoVector::iterator Classes::find( TypeInfo const& type )
    {
      return std::find_if( classes_.begin(), classes_.end(),
        [&type]( ClassInfoVector::value_type const& info )
      {
        return info->type == type;
      } );
    }

    Classes* Classes::instance( operation op, Isolate* isolate )
    {
#if defined(V8PP_ISOLATE_DATA_SLOT)
      Classes* info = static_cast<Classes*>(
        isolate->GetData( V8PP_ISOLATE_DATA_SLOT ) );
      switch ( op )
      {
        case operation::get:
          return info;
        case operation::add:
          if ( !info )
          {
            info = new Classes;
            isolate->SetData( V8PP_ISOLATE_DATA_SLOT, info );
          }
          return info;
        case operation::remove:
          if ( info )
          {
            delete info;
            isolate->SetData( V8PP_ISOLATE_DATA_SLOT, nullptr );
          }
        default:
          return nullptr;
      }
#else
      static std::unordered_map<Isolate*, Classes> instances;
      switch ( op )
      {
        case operation::get:
        {
          auto it = instances.find( isolate );
          return it != instances.end() ? &it->second : nullptr;
        }
        case operation::add:
          return &instances[isolate];
        case operation::remove:
          instances.erase( isolate );
        default:
          return nullptr;
      }
#endif
    }

    template class ObjectRegistry<raw_ptr_traits>;
    template class ObjectRegistry<shared_ptr_traits>;

    template ObjectRegistry<raw_ptr_traits> &Classes::add<raw_ptr_traits>(
      Isolate *isolate, TypeInfo const &type,
      ObjectRegistry<raw_ptr_traits>::dtor_function &&dtor );

    template void Classes::remove<raw_ptr_traits>( Isolate *isolate,
      TypeInfo const &type );

    template ObjectRegistry<raw_ptr_traits> &
      Classes::find<raw_ptr_traits>( Isolate *isolate, TypeInfo const &type );

    template ObjectRegistry<shared_ptr_traits> &Classes::add<shared_ptr_traits>(
      Isolate *isolate, TypeInfo const &type,
      ObjectRegistry<shared_ptr_traits>::dtor_function &&dtor );

    template void Classes::remove<shared_ptr_traits>( Isolate *isolate,
      TypeInfo const &type );

    template ObjectRegistry<shared_ptr_traits> &
      Classes::find<shared_ptr_traits>( Isolate *isolate, TypeInfo const &type );

  }

}