#include "stdafx.h"
#include "js_console.h"
#include "js_util.h"
#include "js_math.h"

namespace neko {

  namespace js {

    const char* c_className = "NekoMath";

    string Math::className( c_className );
    WrappedType Math::internalType = Wrapped_Math;

    struct Messages {
      utf8String syntaxErrorText;
      utf8String nonobjErrorText;
      utf8String nonmatchErrorText;
      Messages( const utf8String& caller )
      {
        syntaxErrorText = "Syntax error: " + caller + "( lhs, rhs )";
        nonobjErrorText = "Usage error: " + caller + "( lhs, rhs ) - unknown object type passed in";
        nonmatchErrorText = "Usage error: " + caller + "( lhs, rhs ) - object types do not match";
      }
    };

    static const Messages c_equalsMessages( c_className + utf8String( ".equals" ) );
    static const Messages c_greaterMessages( c_className + utf8String( ".greaterThan" ) );
    static const Messages c_lesserMessages( c_className + utf8String( ".lessThan" ) );

    Math::Math()
    {
    }

    JSMathPtr Math::create( Isolate* isolate, V8Object global )
    {
      auto instance = make_unique<Math>();
      instance->wrapperRegisterObject( isolate, global );
      return move( instance );
    }

    void Math::registerGlobals( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      JS_WRAPPER_SETOBJMEMBER( tpl, Math, equals );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, equals, eq );
      JS_WRAPPER_SETOBJMEMBER( tpl, Math, greater );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, greater, greaterThan );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, greater, gt );
      JS_WRAPPER_SETOBJMEMBER( tpl, Math, lesser );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, lesser, lessThan );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, lesser, lt );
    }

    bool extractLhsRhsArgObjects( Isolate* isolate, const Messages& msgs, const V8CallbackArgs& args, WrappedType& lhsType, WrappedType& rhsType )
    {
      HandleScope handleScope( isolate );

      if ( args.Length() != 2 )
      {
        util::throwException( isolate, msgs.syntaxErrorText.c_str() );
        return false;
      }

      auto context = args.GetIsolate()->GetCurrentContext();

      if ( !util::getWrappedType( context, args[0], lhsType )
        || !util::getWrappedType( context, args[1], rhsType ) )
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

    //! \verbatim
    //! bool Math.equals( lhs, rhs )
    //! \endverbatim
    void Math::js_equals( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      WrappedType lhsType, rhsType;
      if ( !extractLhsRhsArgObjects( isolate, c_equalsMessages, args, lhsType, rhsType ) )
        return;

      bool retval = false;
      auto context = isolate->GetCurrentContext();
      if ( lhsType == Wrapped_Vector2 )
      {
        auto lhs = extractWrappedDynamic<Vector2>( context, args[0] );
        auto rhs = extractWrappedDynamic<Vector2>( context, args[1] );
        retval = glm::all( glm::equal( lhs->v(), rhs->v() ) );
      }

      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool Math.greater( lhs, rhs )
    //! \endverbatim
    void Math::js_greater( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      WrappedType lhsType, rhsType;
      if ( !extractLhsRhsArgObjects( isolate, c_greaterMessages, args, lhsType, rhsType ) )
        return;

      bool retval = false;
      auto context = isolate->GetCurrentContext();
      if ( lhsType == Wrapped_Vector2 )
      {
        auto lhs = extractWrappedDynamic<Vector2>( context, args[0] );
        auto rhs = extractWrappedDynamic<Vector2>( context, args[1] );
        retval = glm::all( glm::greaterThan( lhs->v(), rhs->v() ) );
      }

      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool Math.lesser( lhs, rhs )
    //! \endverbatim
    void Math::js_lesser( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      WrappedType lhsType, rhsType;
      if ( !extractLhsRhsArgObjects( isolate, c_lesserMessages, args, lhsType, rhsType ) )
        return;

      bool retval = false;
      auto context = isolate->GetCurrentContext();
      if ( lhsType == Wrapped_Vector2 )
      {
        auto lhs = extractWrappedDynamic<Vector2>( context, args[0] );
        auto rhs = extractWrappedDynamic<Vector2>( context, args[1] );
        retval = glm::all( glm::lessThan( lhs->v(), rhs->v() ) );
      }

      args.GetReturnValue().Set( retval );
    }

  }

}