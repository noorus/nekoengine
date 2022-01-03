#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

#include "js_console.h"
#include "js_util.h"
#include "js_math.h"
#include "js_mathutil.h"

namespace neko {

  namespace js {

    namespace mathShared {

      Messages::Messages( const utf8String& caller, bool twoArgs )
      {
        auto argsStr = utf8String( twoArgs ? "( lhs, rhs )" : "( other )" );
        syntaxErrorText = "Syntax error: " + caller + argsStr;
        nonobjErrorText = "Usage error: " + caller + argsStr + " - unknown object type passed in";
        nonmatchErrorText = "Usage error: " + caller + argsStr + " - object types do not match";
      }

    }

    static const char* c_className = "NekoMath";

    static const mathShared::Messages c_equalsMessages( c_className + utf8String( ".equals" ), true );
    static const mathShared::Messages c_greaterMessages( c_className + utf8String( ".greaterThan" ), true );
    static const mathShared::Messages c_greaterOrEqualMessages( c_className + utf8String( ".greaterThanOrEqual" ), true );
    static const mathShared::Messages c_lesserMessages( c_className + utf8String( ".lessThan" ), true );
    static const mathShared::Messages c_lesserOrEqualMessages( c_className + utf8String( ".lessThanOrEqual" ), true );
    static const mathShared::Messages c_addMessages( c_className + utf8String( ".add" ), true );
    static const mathShared::Messages c_subMessages( c_className + utf8String( ".sub" ), true );

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
      // Comparisons - equality
      JS_WRAPPER_SETOBJMEMBER( tpl, Math, equals );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, equals, eq );
      // Comparisons - greater than
      JS_WRAPPER_SETOBJMEMBER( tpl, Math, greater );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, greater, greaterThan );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, greater, gt );
      // Comparisons - greater than or equal
      JS_WRAPPER_SETOBJMEMBER( tpl, Math, greaterOrEqual );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, greaterOrEqual, greaterThanOrEqual );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, greaterOrEqual, gte );
      // Comparisons - less than
      JS_WRAPPER_SETOBJMEMBER( tpl, Math, lesser );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, lesser, lessThan );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, lesser, lt );
      // Comparisons - less than or equal
      JS_WRAPPER_SETOBJMEMBER( tpl, Math, lesserOrEqual );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, lesserOrEqual, lessThanOrEqual );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, lesserOrEqual, lte );
      // Operations - addition
      JS_WRAPPER_SETOBJMEMBER( tpl, Math, add );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, add, addition );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, add, plus );
      // Operations - subtraction
      JS_WRAPPER_SETOBJMEMBER( tpl, Math, sub );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, sub, subtraction );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Math, sub, minus );
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

      bool retval = mathShared::jsmath_greater( isolate, args[0], args[1], lhsType, false );

      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool Math.greaterOrEqual( lhs, rhs )
    //! \endverbatim
    void Math::js_greaterOrEqual( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      WrappedType lhsType, rhsType;
      if ( !mathShared::extractLhsRhsArgsTypes( isolate, c_greaterOrEqualMessages, args, lhsType, rhsType ) )
        return;

      bool retval = mathShared::jsmath_greater( isolate, args[0], args[1], lhsType, true );

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

      bool retval = mathShared::jsmath_lesser( isolate, args[0], args[1], lhsType, false );

      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool Math.lesserOrEqual( lhs, rhs )
    //! \endverbatim
    void Math::js_lesserOrEqual( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      WrappedType lhsType, rhsType;
      if ( !mathShared::extractLhsRhsArgsTypes( isolate, c_lesserOrEqualMessages, args, lhsType, rhsType ) )
        return;

      bool retval = mathShared::jsmath_lesser( isolate, args[0], args[1], lhsType, true );

      args.GetReturnValue().Set( retval );
    }

    //! \verbatim
    //! bool Math.add( lhs, rhs )
    //! \endverbatim
    void Math::js_add( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      auto context = getScriptContext( isolate );
      mathShared::jsmath_add( args, context, c_addMessages, args[0], args[1] );
    }

    //! \verbatim
    //! bool Math.sub( lhs, rhs )
    //! \endverbatim
    void Math::js_sub( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      auto context = getScriptContext( isolate );
      mathShared::jsmath_subtract( args, context, c_addMessages, args[0], args[1] );
    }

  }

}

#endif