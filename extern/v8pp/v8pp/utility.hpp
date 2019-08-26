﻿#pragma once

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

#include <functional>
#include <string>
#include <tuple>
#include <type_traits>

#include "v8pp/config.hpp"

namespace v8pp {
  namespace detail {

    template <typename T>
    struct tuple_tail;

    template <typename Head, typename... Tail>
    struct tuple_tail<std::tuple<Head, Tail...>> {
      using type = std::tuple<Tail...>;
    };

    /////////////////////////////////////////////////////////////////////////////
    //
    // Function traits
    //
    template <typename F>
    struct function_traits;

    template <typename R, typename... Args>
    struct function_traits<R( Args... )> {
      using return_type = R;
      using arguments = std::tuple<Args...>;
    };

    // function pointer
    template <typename R, typename... Args>
    struct function_traits<R( *)( Args... )>
      : function_traits<R( Args... )> {
      using pointer_type = R( *)( Args... );
    };

    // member function pointer
    template <typename C, typename R, typename... Args>
    struct function_traits<R( C::* )( Args... )>
      : function_traits<R( C&, Args... )> {
      template <typename D = C>
      using pointer_type = R( D::* )( Args... );
    };

    // const member function pointer
    template <typename C, typename R, typename... Args>
    struct function_traits<R( C::* )( Args... ) const>
      : function_traits<R( C const&, Args... )> {
      template <typename D = C>
      using pointer_type = R( D::* )( Args... ) const;
    };

    // volatile member function pointer
    template <typename C, typename R, typename... Args>
    struct function_traits<R( C::* )( Args... ) volatile>
      : function_traits<R( C volatile&, Args... )> {
      template <typename D = C>
      using pointer_type = R( D::* )( Args... ) volatile;
    };

    // const volatile member function pointer
    template <typename C, typename R, typename... Args>
    struct function_traits<R( C::* )( Args... ) const volatile>
      : function_traits<R( C const volatile&, Args... )> {
      template <typename D = C>
      using pointer_type = R( D::* )( Args... ) const volatile;
    };

    // member object pointer
    template <typename C, typename R>
    struct function_traits<R( C::* )>
      : function_traits<R( C& )> {
      template <typename D = C>
      using pointer_type = R( D::* );
    };

    // const member object pointer
    template <typename C, typename R>
    struct function_traits<const R( C::* )>
      : function_traits<R( C const& )> {
      template <typename D = C>
      using pointer_type = const R( D::* );
    };

    // volatile member object pointer
    template <typename C, typename R>
    struct function_traits<volatile R( C::* )>
      : function_traits<R( C volatile& )> {
      template <typename D = C>
      using pointer_type = volatile R( D::* );
    };

    // const volatile member object pointer
    template <typename C, typename R>
    struct function_traits<const volatile R( C::* )>
      : function_traits<R( C const volatile& )> {
      template <typename D = C>
      using pointer_type = const volatile R( D::* );
    };

    // function object, std::function, lambda
    template <typename F>
    struct function_traits {
      static_assert( !std::is_bind_expression<F>::value, "std::bind result is not supported yet" );

    private:
      using callable_traits = function_traits<decltype( &F::operator() )>;

    public:
      using return_type = typename callable_traits::return_type;
      using arguments = typename tuple_tail<typename callable_traits::arguments>::type;
    };

    template <typename F>
    struct function_traits<F&>: function_traits<F> {
    };

    template <typename F>
    struct function_traits<F&&>: function_traits<F> {
    };

    template <typename F>
    using is_void_return = std::is_same<void, typename function_traits<F>::return_type>;

    template <typename F, bool is_class>
    struct is_callable_impl
      : std::is_function<typename std::remove_pointer<F>::type> {
    };

    template <typename F>
    struct is_callable_impl<F, true> {
    private:
      struct fallback {
        void operator()();
      };
      struct derived: F, fallback {
      };

      template <typename U, U>
      struct check;

      template <typename>
      static std::true_type test( ... );

      template <typename C>
      static std::false_type test( check<void ( fallback::* )( ), &C::operator()>* );

      using type = decltype( test<derived>( 0 ) );

    public:
      static const bool value = type::value;
    };

    template <typename F>
    using is_callable = std::integral_constant<bool, is_callable_impl<F, std::is_class<F>::value>::value>;

    using std::index_sequence;
    using std::make_index_sequence;

    /// Type information for custom RTTI
    class TypeInfo {
    public:
      inline utf8String const& name() const { return name_; }
      bool operator==( TypeInfo const& other ) const { return name_ == other.name_; }
      bool operator!=( TypeInfo const& other ) const { return name_ != other.name_; }

    private:
      template <typename T>
      friend TypeInfo type_id();
      TypeInfo( char const* name, size_t size )
        : name_( name, size )
      {
      }
      utf8String name_;
    };

    /// Get type information for type T
    /// The idea is borrowed from https://github.com/Manu343726/ctti
    template <typename T>
    TypeInfo type_id()
    {
#if defined( _MSC_VER )
#define V8PP_PRETTY_FUNCTION __FUNCSIG__
#define V8PP_PRETTY_FUNCTION_PREFIX "class v8pp::detail::type_info __cdecl v8pp::detail::type_id<"
#define V8PP_PRETTY_FUNCTION_SUFFIX ">(void)"
#elif defined( __clang__ ) || defined( __GNUC__ )
#define V8PP_PRETTY_FUNCTION __PRETTY_FUNCTION__
#if !defined( __clang__ )
#define V8PP_PRETTY_FUNCTION_PREFIX "v8pp::detail::type_info v8pp::detail::type_id() [with T = "
#else
#define V8PP_PRETTY_FUNCTION_PREFIX "v8pp::detail::type_info v8pp::detail::type_id() [T = "
#endif
#define V8PP_PRETTY_FUNCTION_SUFFIX "]"
#else
#error "Unknown compiler"
#endif

#define V8PP_PRETTY_FUNCTION_LEN ( sizeof( V8PP_PRETTY_FUNCTION ) - 1 )
#define V8PP_PRETTY_FUNCTION_PREFIX_LEN ( sizeof( V8PP_PRETTY_FUNCTION_PREFIX ) - 1 )
#define V8PP_PRETTY_FUNCTION_SUFFIX_LEN ( sizeof( V8PP_PRETTY_FUNCTION_SUFFIX ) - 1 )

      return TypeInfo( V8PP_PRETTY_FUNCTION + V8PP_PRETTY_FUNCTION_PREFIX_LEN, V8PP_PRETTY_FUNCTION_LEN - V8PP_PRETTY_FUNCTION_PREFIX_LEN - V8PP_PRETTY_FUNCTION_SUFFIX_LEN );

#undef V8PP_PRETTY_FUNCTION
#undef V8PP_PRETTY_FUNCTION_PREFIX
#undef V8PP_PRETTY_FUNCTION_SUFFFIX
#undef V8PP_PRETTY_FUNCTION_LEN
#undef V8PP_PRETTY_FUNCTION_PREFIX_LEN
#undef V8PP_PRETTY_FUNCTION_SUFFFIX_LEN
    }

  }
}