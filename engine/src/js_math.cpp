#include "stdafx.h"
#include "js_console.h"
#include "js_util.h"
#include "js_math.h"
#include "js_mathutil.h"

namespace neko {

  namespace js {

    namespace mathShared {

      Messages::Messages( const utf8String& caller )
      {
        syntaxErrorText = "Syntax error: " + caller + "( lhs, rhs )";
        nonobjErrorText = "Usage error: " + caller + "( lhs, rhs ) - unknown object type passed in";
        nonmatchErrorText = "Usage error: " + caller + "( lhs, rhs ) - object types do not match";
      }

    }

    const char* c_className = "NekoMath";

    static const mathShared::Messages c_equalsMessages( c_className + utf8String( ".equals" ) );
    static const mathShared::Messages c_greaterMessages( c_className + utf8String( ".greaterThan" ) );
    static const mathShared::Messages c_lesserMessages( c_className + utf8String( ".lessThan" ) );

    string Math::className( c_className );
    WrappedType Math::internalType = Wrapped_Math;

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

    //! \verbatim
    //! bool Math.equals( lhs, rhs )
    //! \endverbatim
    void Math::js_equals( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      WrappedType lhsType, rhsType;
      if ( !mathShared::extractLhsRhsArgsTypes( isolate, c_equalsMessages, args, lhsType, rhsType ) )
        return;

      bool retval = mathShared::jsmath_equals( isolate, args[0], args[1], lhsType );

      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool Math.greater( lhs, rhs )
    //! \endverbatim
    void Math::js_greater( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      WrappedType lhsType, rhsType;
      if ( !mathShared::extractLhsRhsArgsTypes( isolate, c_greaterMessages, args, lhsType, rhsType ) )
        return;

      bool retval = mathShared::jsmath_greater( isolate, args[0], args[1], lhsType );

      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool Math.lesser( lhs, rhs )
    //! \endverbatim
    void Math::js_lesser( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      WrappedType lhsType, rhsType;
      if ( !mathShared::extractLhsRhsArgsTypes( isolate, c_lesserMessages, args, lhsType, rhsType ) )
        return;

      bool retval = mathShared::jsmath_lesser( isolate, args[0], args[1], lhsType );

      args.GetReturnValue().Set( retval );
    }

  }

}