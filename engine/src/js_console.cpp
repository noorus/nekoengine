#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

# include "js_console.h"
# include "js_util.h"
# include "console.h"

namespace neko {

  namespace js {

    string StaticObjectWrapper<Console>::className( "Console" );
    WrappedType StaticObjectWrapper<Console>::internalType = Wrapped_Console;

    Console::Console( ConsolePtr console ): console_( move( console ) ) {}

    JSConsolePtr Console::create( ConsolePtr console, Isolate* isolate, V8Object global )
    {
      auto instance = make_unique<Console>( move( console ) );
      instance->wrapperRegisterObject( isolate, global );
      return move( instance );
    }

    void Console::registerGlobals( Isolate* isolate, V8FunctionTemplate& tpl )
    {
      JS_WRAPPER_SETOBJMEMBER( tpl, Console, print );
      JS_WRAPPER_SETOBJMEMBERNAMED( tpl, Console, print, log );
      JS_WRAPPER_SETOBJMEMBER( tpl, Console, getVariable );
      JS_WRAPPER_SETOBJMEMBER( tpl, Console, setVariable );
      JS_WRAPPER_SETOBJMEMBER( tpl, Console, execute );
    }

    void Console::js_print( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      utf8String msg;
      for ( int i = 0; i < args.Length(); i++ )
      {
        if ( i > 0 )
          msg.append( " " );
        v8::String::Utf8Value str( isolate, args[i] );
        if ( *str )
          msg.append( *str );
      }

      console_->print( neko::Console::srcScripting, msg );
    }

    void Console::js_getVariable( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 || !args[0]->IsString() )
      {
        util::throwException( isolate, "Syntax error: String Console.getVariable( String variable )" );
        return;
      }

      v8::String::Utf8Value variableName( args.GetIsolate(), args[0] );

      ConVar* variable = console_->getVariable( (const char*)*variableName );

      if ( !variable )
      {
        util::throwException( isolate, L"Console.getVariable: No such console variable \"%s\"", (const char*)*variableName );
        return;
      }

      args.GetReturnValue().Set( util::allocString( variable->as_s(), isolate ) );
    }

    void Console::js_setVariable( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      if ( args.Length() != 2 || !args[0]->IsString() )
      {
        util::throwException( isolate, "Syntax error: Console.setVariable( String variable, String value )" );
        return;
      }

      v8::String::Utf8Value variableName( isolate, args[0] );
      v8::String::Utf8Value variableValue( isolate, args[1] );

      ConVar* variable = console_->getVariable( (const char*)*variableName );

      if ( !variable )
      {
        util::throwException( isolate, L"Console.setVariable: No such console variable \"%s\"", (const wchar_t*)*variableName );
        return;
      }

      variable->set( (const char*)*variableValue );
    }

    void Console::js_execute( Isolate* isolate, const V8CallbackArgs& args )
    {
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 || !args[0]->IsString() )
      {
        util::throwException( isolate, "Syntax error: Console.execute( String commandLine )" );
        return;
      }

      v8::String::Utf8Value commandLine( isolate, args[0] );

      console_->execute( (const char*)*commandLine );
    }

  }

}

#endif