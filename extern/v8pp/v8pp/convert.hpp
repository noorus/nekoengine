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

#include <climits>
#include <string>
#include <array>
#include <vector>
#include <map>
#include <memory>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <v8.h>

#include "v8pp/config.hpp"
#include "v8pp/ptr_traits.hpp"

namespace v8pp {

  template <typename T, typename Traits>
  class class_;

  // Generic convertor
  /*
template<typename T, typename Enable = void>
struct convert
{
  using from_type = T;
  using to_type = Local<Value>;

  static bool is_valid(Isolate* isolate, Local<Value> value);

  static from_type from_v8(Isolate* isolate, Local<Value> value);
  static to_type to_v8(Isolate* isolate, T const& value);
};
*/

  struct invalid_argument: std::invalid_argument {
    invalid_argument( Isolate* isolate, Local<Value> value, char const* expected_type );
  };

  // converter specializations for string types
  template <typename Char, typename Traits, typename Alloc>
  struct convert<std::basic_string<Char, Traits, Alloc>> {
    static_assert( sizeof( Char ) <= sizeof( uint16_t ), "only UTF-8 and UTF-16 strings are supported" );

    using from_type = std::basic_string<Char, Traits, Alloc>;
    using to_type = Local<String>;

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.IsEmpty() && value->IsString();
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        throw invalid_argument( isolate, value, "String" );
      }

      if ( sizeof( Char ) == 1 )
      {
        String::Utf8Value const str( isolate, value );
        return from_type( reinterpret_cast<Char const*>( *str ), str.length() );
      } else
      {
        String::Value const str( isolate, value );
        return from_type( reinterpret_cast<Char const*>( *str ), str.length() );
      }
    }

    static to_type to_v8( Isolate* isolate, from_type const& value )
    {
      if ( sizeof( Char ) == 1 )
      {
        return String::NewFromUtf8( isolate, reinterpret_cast<char const*>( value.data() ), v8::NewStringType::kNormal, static_cast<int>( value.length() ) ).ToLocalChecked();
      } else
      {
        return String::NewFromTwoByte( isolate, reinterpret_cast<uint16_t const*>( value.data() ), v8::NewStringType::kNormal, static_cast<int>( value.length() ) ).ToLocalChecked();
      }
    }
  };

  template <typename Char>
  struct convert<Char const*, typename std::enable_if<std::is_same<Char, char>::value || std::is_same<Char, char16_t>::value || std::is_same<Char, wchar_t>::value>::type> {
    static_assert( sizeof( Char ) <= sizeof( uint16_t ), "only UTF-8 and UTF-16 strings are supported" );

    // A string that converts to Char const *
    struct convertible_string: std::basic_string<Char> {
      convertible_string( Char const* str, size_t len )
        : std::basic_string<Char>( str, len ) {}
      operator Char const*( ) const { return this->c_str(); }
    };

    using from_type = convertible_string;
    using to_type = Local<String>;

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.IsEmpty() && value->IsString();
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        throw invalid_argument( isolate, value, "String" );
      }

      if ( sizeof( Char ) == 1 )
      {
        String::Utf8Value const str( isolate, value );
        return from_type( reinterpret_cast<Char const*>( *str ), str.length() );
      } else
      {
        String::Value const str( isolate, value );
        return from_type( reinterpret_cast<Char const*>( *str ), str.length() );
      }
    }

    static to_type to_v8( Isolate* isolate, Char const* value, size_t len = ~0 )
    {
      if ( sizeof( Char ) == 1 )
      {
        return String::NewFromUtf8( isolate, reinterpret_cast<char const*>( value ), v8::NewStringType::kNormal, static_cast<int>( len ) ).ToLocalChecked();
      } else
      {
        return String::NewFromTwoByte( isolate, reinterpret_cast<uint16_t const*>( value ), v8::NewStringType::kNormal, static_cast<int>( len ) ).ToLocalChecked();
      }
    }
  };

  // converter specializations for primitive types
  template <>
  struct convert<bool> {
    using from_type = bool;
    using to_type = Local<Boolean>;

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.IsEmpty() && value->IsBoolean();
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        throw invalid_argument( isolate, value, "Boolean" );
      }
      return value->BooleanValue( isolate );
    }

    static to_type to_v8( Isolate* isolate, bool value )
    {
      return Boolean::New( isolate, value );
    }
  };

  template <typename T>
  struct convert<T, typename std::enable_if<std::is_integral<T>::value>::type> {
    using from_type = T;
    using to_type = Local<Number>;

    enum {
      bits = sizeof( T ) * CHAR_BIT,
      is_signed = std::is_signed<T>::value
    };

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.IsEmpty() && value->IsNumber();
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        throw invalid_argument( isolate, value, "Number" );
      }

      if ( bits <= 32 )
      {
        if ( is_signed )
        {
          return static_cast<T>( value->Int32Value( isolate->GetCurrentContext() ).FromJust() );
        } else
        {
          return static_cast<T>( value->Uint32Value( isolate->GetCurrentContext() ).FromJust() );
        }
      } else
      {
        return static_cast<T>( value->IntegerValue( isolate->GetCurrentContext() ).FromJust() );
      }
    }

    static to_type to_v8( Isolate* isolate, T value )
    {
      if ( bits <= 32 )
      {
        if ( is_signed )
        {
          return Integer::New( isolate, static_cast<int32_t>( value ) );
        } else
        {
          return Integer::NewFromUnsigned( isolate, static_cast<uint32_t>( value ) );
        }
      } else
      {
        //TODO: check value < (1<<57) to fit in double?
        return Number::New( isolate, static_cast<double>( value ) );
      }
    }
  };

  template <typename T>
  struct convert<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    using underlying_type = typename std::underlying_type<T>::type;

    using from_type = T;
    using to_type = typename convert<underlying_type>::to_type;

    static bool is_valid( Isolate* isolate, Local<Value> value )
    {
      return convert<underlying_type>::is_valid( isolate, value );
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      return static_cast<T>( convert<underlying_type>::from_v8( isolate, value ) );
    }

    static to_type to_v8( Isolate* isolate, T value )
    {
      return convert<underlying_type>::to_v8( isolate, static_cast<underlying_type>( value ) );
    }
  };

  template <typename T>
  struct convert<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    using from_type = T;
    using to_type = Local<Number>;

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.IsEmpty() && value->IsNumber();
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        throw invalid_argument( isolate, value, "Number" );
      }

      return static_cast<T>( value->NumberValue( isolate->GetCurrentContext() ).FromJust() );
    }

    static to_type to_v8( Isolate* isolate, T value )
    {
      return Number::New( isolate, value );
    }
  };

  // convert Array <-> std::array
  template <typename T, size_t N>
  struct convert<std::array<T, N>> {
    using from_type = std::array<T, N>;
    using to_type = Local<Array>;

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.IsEmpty() && value->IsArray();
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        throw invalid_argument( isolate, value, "Array" );
      }

      HandleScope scope( isolate );
      Local<Context> context = isolate->GetCurrentContext();
      Local<Array> array = value.As<Array>();

      if ( array->Length() != N )
      {
        throw std::runtime_error( "Invalid array length: expected " + std::to_string( N ) + " actual " + std::to_string( array->Length() ) );
      }

      from_type result;
      for ( uint32_t i = 0; i < N; ++i )
      {
        result[i] = convert<T>::from_v8( isolate, array->Get( context, i ).ToLocalChecked() );
      }
      return result;
    }

    static to_type to_v8( Isolate* isolate, from_type const& value )
    {
      EscapableHandleScope scope( isolate );
      Local<Context> context = isolate->GetCurrentContext();
      Local<Array> result = Array::New( isolate, N );
      for ( uint32_t i = 0; i < N; ++i )
      {
        result->Set( context, i, convert<T>::to_v8( isolate, value[i] ) ).FromJust();
      }
      return scope.Escape( result );
    }
  };

  // convert Array <-> std::vector
  template <typename T, typename Alloc>
  struct convert<std::vector<T, Alloc>> {
    using from_type = std::vector<T, Alloc>;
    using to_type = Local<Array>;

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.IsEmpty() && value->IsArray();
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        throw invalid_argument( isolate, value, "Array" );
      }

      HandleScope scope( isolate );
      Local<Context> context = isolate->GetCurrentContext();
      Local<Array> array = value.As<Array>();

      from_type result;
      result.reserve( array->Length() );
      for ( uint32_t i = 0, count = array->Length(); i < count; ++i )
      {
        result.emplace_back( convert<T>::from_v8( isolate, array->Get( context, i ).ToLocalChecked() ) );
      }
      return result;
    }

    static to_type to_v8( Isolate* isolate, from_type const& value )
    {
      EscapableHandleScope scope( isolate );
      Local<Context> context = isolate->GetCurrentContext();
      uint32_t const size = static_cast<uint32_t>( value.size() );
      Local<Array> result = Array::New( isolate, size );
      for ( uint32_t i = 0; i < size; ++i )
      {
        result->Set( context, i, convert<T>::to_v8( isolate, value[i] ) ).FromJust();
      }
      return scope.Escape( result );
    }
  };

  // convert Object <-> std::map
  template <typename Key, typename Value, typename Less, typename Alloc>
  struct convert<std::map<Key, Value, Less, Alloc>> {
    using from_type = std::map<Key, Value, Less, Alloc>;
    using to_type = Local<Object>;

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.IsEmpty() && value->IsObject();
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        throw invalid_argument( isolate, value, "Object" );
      }

      HandleScope scope( isolate );
      Local<Context> context = isolate->GetCurrentContext();
      Local<Object> object = value.As<Object>();
      Local<Array> prop_names = object->GetPropertyNames( context ).ToLocalChecked();

      from_type result;
      for ( uint32_t i = 0, count = prop_names->Length(); i < count; ++i )
      {
        Local<Value> key = prop_names->Get( context, i ).ToLocalChecked();
        Local<Value> val = object->Get( context, key ).ToLocalChecked();
        result.emplace( convert<Key>::from_v8( isolate, key ), convert<Value>::from_v8( isolate, val ) );
      }
      return result;
    }

    static to_type to_v8( Isolate* isolate, from_type const& value )
    {
      EscapableHandleScope scope( isolate );
      Local<Context> context = isolate->GetCurrentContext();
      Local<Object> result = Object::New( isolate );
      for ( auto const& item : value )
      {
        result->Set( context, convert<Key>::to_v8( isolate, item.first ), convert<Value>::to_v8( isolate, item.second ) ).FromJust();
      }
      return scope.Escape( result );
    }
  };

  template <typename T>
  struct convert<Local<T>> {
    using from_type = Local<T>;
    using to_type = Local<T>;

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.As<T>().IsEmpty();
    }

    static Local<T> from_v8( Isolate*, Local<Value> value )
    {
      return value.As<T>();
    }

    static Local<T> to_v8( Isolate*, Local<T> value )
    {
      return value;
    }
  };

  template <typename T>
  struct is_wrapped_class: std::is_class<T> {
  };

  // convert specialization for wrapped user classes
  template <typename T>
  struct is_wrapped_class<Local<T>>: std::false_type {
  };

  template <typename T>
  struct is_wrapped_class<Global<T>>: std::false_type {
  };

  template <typename Char, typename Traits, typename Alloc>
  struct is_wrapped_class<std::basic_string<Char, Traits, Alloc>>: std::false_type {
  };

  template <typename T, size_t N>
  struct is_wrapped_class<std::array<T, N>>: std::false_type {
  };

  template <typename T, typename Alloc>
  struct is_wrapped_class<std::vector<T, Alloc>>: std::false_type {
  };

  template <typename Key, typename Value, typename Less, typename Alloc>
  struct is_wrapped_class<std::map<Key, Value, Less, Alloc>>: std::false_type {
  };

  template <typename T>
  struct is_wrapped_class<std::shared_ptr<T>>: std::false_type {
  };

  template <typename T>
  struct convert<T*, typename std::enable_if<is_wrapped_class<T>::value>::type>
  {
    using from_type = T * ;
    using to_type = Local<Object>;
    using class_type = typename std::remove_cv<T>::type;

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.IsEmpty() && value->IsObject();
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        return nullptr;
      }
      return class_<class_type, raw_ptr_traits>::unwrap( isolate, value );
    }

    static to_type to_v8( Isolate* isolate, T const* value )
    {
      return class_<class_type, raw_ptr_traits>::find_object( isolate, value );
    }
  };

  template <typename T>
  struct convert<T, typename std::enable_if<is_wrapped_class<T>::value>::type>
  {
    using from_type = T & ;
    using to_type = Local<Object>;
    using class_type = typename std::remove_cv<T>::type;

    static bool is_valid( Isolate* isolate, Local<Value> value )
    {
      return convert<T*>::is_valid( isolate, value );
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        throw invalid_argument( isolate, value, "Object" );
      }
      T* object = class_<class_type, raw_ptr_traits>::unwrap( isolate, value );
      if ( object )
      {
        return *object;
      }
      throw std::runtime_error( "failed to unwrap C++ object" );
    }

    static to_type to_v8( Isolate* isolate, T const& value )
    {
      Local<Object> result = class_<class_type, raw_ptr_traits>::find_object( isolate, value );
      if ( !result.IsEmpty() )
        return result;
      throw std::runtime_error( "failed to wrap C++ object" );
    }
  };

  template <typename T>
  struct convert<std::shared_ptr<T>, typename std::enable_if<is_wrapped_class<T>::value>::type> {
    using from_type = std::shared_ptr<T>;
    using to_type = Local<Object>;
    using class_type = typename std::remove_cv<T>::type;

    static bool is_valid( Isolate*, Local<Value> value )
    {
      return !value.IsEmpty() && value->IsObject();
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        return nullptr;
      }
      return class_<class_type, shared_ptr_traits>::unwrap( isolate, value );
    }

    static to_type to_v8( Isolate* isolate, std::shared_ptr<T> const& value )
    {
      return class_<class_type, shared_ptr_traits>::find_object( isolate, value );
    }
  };

  template <typename T>
  struct convert<T, ref_from_shared_ptr> {
    using from_type = T & ;
    using to_type = Local<Object>;
    using class_type = typename std::remove_cv<T>::type;

    static bool is_valid( Isolate* isolate, Local<Value> value )
    {
      return convert<std::shared_ptr<T>>::is_valid( isolate, value );
    }

    static from_type from_v8( Isolate* isolate, Local<Value> value )
    {
      if ( !is_valid( isolate, value ) )
      {
        throw invalid_argument( isolate, value, "Object" );
      }
      std::shared_ptr<T> object = class_<class_type, shared_ptr_traits>::unwrap( isolate, value );
      if ( object )
      {
        //			assert(object.use_count() > 1);
        return *object;
      }
      throw std::runtime_error( "failed to unwrap C++ object" );
    }

    static to_type to_v8( Isolate* isolate, T const& value )
    {
      Local<Object> result = class_<class_type, shared_ptr_traits>::find_object( isolate, value );
      if ( !result.IsEmpty() )
        return result;
      throw std::runtime_error( "failed to wrap C++ object" );
    }
  };

  template <typename T>
  struct convert<T&>: convert<T> {
  };

  template <typename T>
  struct convert<T const&>: convert<T> {
  };

  template <typename T>
  auto from_v8( Isolate* isolate, Local<Value> value )
    -> decltype( convert<T>::from_v8( isolate, value ) )
  {
    return convert<T>::from_v8( isolate, value );
  }

  template <typename T, typename U>
  auto from_v8( Isolate* isolate, Local<Value> value, U const& default_value )
    -> decltype( convert<T>::from_v8( isolate, value ) )
  {
    return convert<T>::is_valid( isolate, value ) ?
      convert<T>::from_v8( isolate, value ) :
      default_value;
  }

  inline Local<String> to_v8( Isolate* isolate, char const* str, size_t len )
  {
    return convert<char const*>::to_v8( isolate, str, len );
  }

  template <size_t N>
  Local<String> to_v8( Isolate* isolate, char const ( &str )[N], size_t len = N - 1 )
  {
    return convert<char const*>::to_v8( isolate, str, len );
  }

  inline Local<String> to_v8( Isolate* isolate, char16_t const* str, size_t len )
  {
    return convert<char16_t const*>::to_v8( isolate, str, len );
  }

  template <size_t N>
  Local<String> to_v8( Isolate* isolate, char16_t const ( &str )[N], size_t len = N - 1 )
  {
    return convert<char16_t const*>::to_v8( isolate, str, len );
  }

#ifdef WIN32
  inline Local<String> to_v8( Isolate* isolate, wchar_t const* str, size_t len )
  {
    return convert<wchar_t const*>::to_v8( isolate, str, len );
  }

  template <size_t N>
  Local<String> to_v8( Isolate* isolate, wchar_t const ( &str )[N], size_t len = N - 1 )
  {
    return convert<wchar_t const*>::to_v8( isolate, str, len );
  }
#endif

  template <typename T>
  auto to_v8( Isolate* isolate, T const& value )
    -> decltype( convert<T>::to_v8( isolate, value ) )
  {
    return convert<T>::to_v8( isolate, value );
  }

  template <typename Iterator>
  Local<Array> to_v8( Isolate* isolate, Iterator begin, Iterator end )
  {
    EscapableHandleScope scope( isolate );
    Local<Context> context = isolate->GetCurrentContext();
    Local<Array> result = Array::New( isolate );
    for ( uint32_t idx = 0; begin != end; ++begin, ++idx )
    {
      result->Set( context, idx, to_v8( isolate, *begin ) ).FromJust();
    }
    return scope.Escape( result );
  }

  template <typename T>
  Local<Array> to_v8( Isolate* isolate, std::initializer_list<T> const& init )
  {
    return to_v8( isolate, init.begin(), init.end() );
  }

  template <typename T>
  Local<T> to_local( Isolate* isolate, PersistentBase<T> const& handle )
  {
    if ( handle.IsWeak() )
    {
      return Local<T>::New( isolate, handle );
    } else
    {
      return *reinterpret_cast<Local<T>*>(
        const_cast<PersistentBase<T>*>( &handle ) );
    }
  }

  inline invalid_argument::invalid_argument( Isolate* isolate, Local<Value> value, char const* expected_type )
    : std::invalid_argument( std::string( "expected " ) + expected_type + ", typeof=" + ( value.IsEmpty() ? std::string( "<empty>" ) : v8pp::from_v8<std::string>( isolate, value->TypeOf( isolate ) ) ) )
  {
  }

}