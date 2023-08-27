#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "js_math.h"
#include "scripting.h"

namespace neko {

  namespace js {

    namespace mathShared {

      struct Messages {
        utf8String syntaxErrorText;
        utf8String nonobjErrorText;
        utf8String nonmatchErrorText;
        Messages( const utf8String& caller, bool twoArgs );
      };

      inline bool extractLhsRhsTypes( Isolate* isolate, const Messages& msgs, const V8Value& lhs, const V8Value& rhs, WrappedType& lhsType, WrappedType& rhsType, bool mustMatch = true )
      {
        HandleScope handleScope( isolate );

        auto context = isolate->GetCurrentContext();

        if ( !util::getWrappedType( context, lhs, lhsType )
          || !util::getWrappedType( context, rhs, rhsType ) )
        {
          util::throwException( isolate, msgs.nonobjErrorText.c_str() );
          return false;
        }

        if ( mustMatch && lhsType != rhsType )
        {
          util::throwException( isolate, msgs.nonmatchErrorText.c_str() );
          return false;
        }

        return true;
      }

      inline bool extractLhsRhsArgsTypes( Isolate* isolate, const Messages& msgs, const V8CallbackArgs& args, WrappedType& lhsType, WrappedType& rhsType, bool mustMatch = true )
      {
        HandleScope handleScope( isolate );

        if ( args.Length() != 2 )
        {
          util::throwException( isolate, msgs.syntaxErrorText.c_str() );
          return false;
        }

        if ( !extractLhsRhsTypes( isolate, msgs, args[0], args[1], lhsType, rhsType, mustMatch ) )
          return false;

        return true;
      }

      inline bool jsmath_equals( Isolate* isolate, const V8Value& lhs, const V8Value& rhs, const WrappedType type )
      {
        HandleScope handleScope( isolate );

        bool retval = false;
        auto context = isolate->GetCurrentContext();
        if ( type == Wrapped_Vector2 )
        {
          auto lobj = extractWrappedDynamic<Vector2>( context, lhs );
          auto robj = extractWrappedDynamic<Vector2>( context, rhs );
          retval = glm::all( glm::equal( lobj->v(), robj->v() ) );
        }
        else if ( type == Wrapped_Vector3 )
        {
          auto lobj = extractWrappedDynamic<Vector3>( context, lhs );
          auto robj = extractWrappedDynamic<Vector3>( context, rhs );
          retval = glm::all( glm::equal( lobj->v(), robj->v() ) );
        }

        return retval;
      }

      inline bool jsmath_greater( Isolate* isolate, const V8Value& lhs, const V8Value& rhs, const WrappedType type, bool orEqual = false )
      {
        HandleScope handleScope( isolate );

        bool retval = false;
        auto context = isolate->GetCurrentContext();
        if ( type == Wrapped_Vector2 )
        {
          auto lobj = extractWrappedDynamic<Vector2>( context, lhs );
          auto robj = extractWrappedDynamic<Vector2>( context, rhs );
          retval = glm::all( orEqual ? glm::greaterThanEqual( lobj->v(), robj->v() ) : glm::greaterThan( lobj->v(), robj->v() ) );
        }
        else if ( type == Wrapped_Vector3 )
        {
          auto lobj = extractWrappedDynamic<Vector3>( context, lhs );
          auto robj = extractWrappedDynamic<Vector3>( context, rhs );
          retval = glm::all( orEqual ? glm::greaterThanEqual( lobj->v(), robj->v() ) : glm::greaterThan( lobj->v(), robj->v() ) );
        }

        return retval;
      }

      inline bool jsmath_lesser( Isolate* isolate, const V8Value& lhs, const V8Value& rhs, const WrappedType type, bool orEqual = false )
      {
        HandleScope handleScope( isolate );

        bool retval = false;
        auto context = isolate->GetCurrentContext();
        if ( type == Wrapped_Vector2 )
        {
          auto lobj = extractWrappedDynamic<Vector2>( context, lhs );
          auto robj = extractWrappedDynamic<Vector2>( context, rhs );
          retval = glm::all( orEqual ? glm::lessThanEqual( lobj->v(), robj->v() ) : glm::lessThan( lobj->v(), robj->v() ) );
        }
        else if ( type == Wrapped_Vector3 )
        {
          auto lobj = extractWrappedDynamic<Vector3>( context, lhs );
          auto robj = extractWrappedDynamic<Vector3>( context, rhs );
          retval = glm::all( orEqual ? glm::lessThanEqual( lobj->v(), robj->v() ) : glm::lessThan( lobj->v(), robj->v() ) );
        }

        return retval;
      }

      template <typename T>
      inline shared_ptr<T> inl_addition( V8Context& context, const v8::ReturnValue<v8::Value>& ret, typename T::RegistryPtrType& registry, const V8Value& lhs, const V8Value& rhs )
      {
        auto lobj = extractWrappedDynamic<T>( context, lhs );
        auto robj = extractWrappedDynamic<T>( context, rhs );
        auto sum = ( lobj->v() + robj->v() );
        return registry->createFrom( sum );
      }

      template <typename T>
      inline shared_ptr<T> inl_subtraction( V8Context& context, const v8::ReturnValue<v8::Value>& ret,
        typename T::RegistryPtrType& registry, const V8Value& lhs, const V8Value& rhs )
      {
        auto lobj = extractWrappedDynamic<T>( context, lhs );
        auto robj = extractWrappedDynamic<T>( context, rhs );
        auto sum = ( lobj->v() - robj->v() );
        return registry->createFrom( sum );
      }

      inline bool jsmath_add( const V8CallbackArgs& args, ScriptingContext* ctx, const Messages& msgs, const V8Value& lhs, const V8Value& rhs )
      {
        auto isolate = args.GetIsolate();
        HandleScope handleScope( isolate );

        auto context = isolate->GetCurrentContext();

        WrappedType lhsType, rhsType;
        if ( !extractLhsRhsTypes( isolate, msgs, lhs, rhs, lhsType, rhsType, false ) )
          return false;

        if ( lhsType == Wrapped_Vector2 && rhsType == Wrapped_Vector2 )
        {
          auto ptr = inl_addition<Vector2>( context, args.GetReturnValue(), ctx->vec2reg(), lhs, rhs );
          args.GetReturnValue().Set( ptr->handle( context->GetIsolate() ) );
          return true;
        }
        else if ( lhsType == Wrapped_Vector3 && rhsType == Wrapped_Vector3 )
        {
          auto ptr = inl_addition<Vector3>( context, args.GetReturnValue(), ctx->vec3reg(), lhs, rhs );
          args.GetReturnValue().Set( ptr->handle( context->GetIsolate() ) );
          return true;
        }

        return false;
      }

      inline bool jsmath_subtract( const V8CallbackArgs& args, ScriptingContext* ctx, const Messages& msgs, const V8Value& lhs, const V8Value& rhs )
      {
        auto isolate = args.GetIsolate();
        HandleScope handleScope( isolate );

        auto context = isolate->GetCurrentContext();

        WrappedType lhsType, rhsType;
        if ( !extractLhsRhsTypes( isolate, msgs, lhs, rhs, lhsType, rhsType, false ) )
          return false;

        if ( lhsType == Wrapped_Vector2 && rhsType == Wrapped_Vector2 )
        {
          auto ptr = inl_subtraction<Vector2>( context, args.GetReturnValue(), ctx->vec2reg(), lhs, rhs );
          args.GetReturnValue().Set( ptr->handle( context->GetIsolate() ) );
          return true;
        }
        else if ( lhsType == Wrapped_Vector3 && rhsType == Wrapped_Vector3 )
        {
          auto ptr = inl_subtraction<Vector3>( context, args.GetReturnValue(), ctx->vec3reg(), lhs, rhs );
          args.GetReturnValue().Set( ptr->handle( context->GetIsolate() ) );
          return true;
        }

        return false;
      }

    }

  }

}

#endif