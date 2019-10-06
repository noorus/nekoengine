#pragma once
#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "js_wrapper.h"
#include "js_math.h"

namespace neko {

  namespace js {

    namespace mathShared {

      struct Messages {
        utf8String syntaxErrorText;
        utf8String nonobjErrorText;
        utf8String nonmatchErrorText;
        Messages( const utf8String& caller );
      };

      inline bool extractLhsRhsTypes( Isolate* isolate, const Messages& msgs, const V8Value& lhs, const V8Value& rhs, WrappedType& lhsType, WrappedType& rhsType )
      {
        HandleScope handleScope( isolate );

        auto context = isolate->GetCurrentContext();

        if ( !util::getWrappedType( context, lhs, lhsType )
          || !util::getWrappedType( context, rhs, rhsType ) )
        {
          util::throwException( isolate, msgs.nonobjErrorText.c_str() );
          return false;
        }

        if ( lhsType != rhsType )
        {
          util::throwException( isolate, msgs.nonmatchErrorText.c_str() );
          return false;
        }

        return true;
      }

      inline bool extractLhsRhsArgsTypes( Isolate* isolate, const Messages& msgs, const V8CallbackArgs& args, WrappedType& lhsType, WrappedType& rhsType )
      {
        HandleScope handleScope( isolate );

        if ( args.Length() != 2 )
        {
          util::throwException( isolate, msgs.syntaxErrorText.c_str() );
          return false;
        }

        if ( !extractLhsRhsTypes( isolate, msgs, args[0], args[1], lhsType, rhsType ) )
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

        return retval;
      }

      inline bool jsmath_greater( Isolate* isolate, const V8Value& lhs, const V8Value& rhs, const WrappedType type )
      {
        HandleScope handleScope( isolate );

        bool retval = false;
        auto context = isolate->GetCurrentContext();
        if ( type == Wrapped_Vector2 )
        {
          auto lobj = extractWrappedDynamic<Vector2>( context, lhs );
          auto robj = extractWrappedDynamic<Vector2>( context, rhs );
          retval = glm::all( glm::greaterThan( lobj->v(), robj->v() ) );
        }

        return retval;
      }

      inline bool jsmath_lesser( Isolate* isolate, const V8Value& lhs, const V8Value& rhs, const WrappedType type )
      {
        HandleScope handleScope( isolate );

        bool retval = false;
        auto context = isolate->GetCurrentContext();
        if ( type == Wrapped_Vector2 )
        {
          auto lobj = extractWrappedDynamic<Vector2>( context, lhs );
          auto robj = extractWrappedDynamic<Vector2>( context, rhs );
          retval = glm::all( glm::lessThan( lobj->v(), robj->v() ) );
        }

        return retval;
      }

    }

  }

}