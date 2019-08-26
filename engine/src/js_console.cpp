#include "stdafx.h"
#include "js_console.h"
#include "js_util.h"
#include "console.h"

namespace neko {

  namespace js {

    string Console::className( "Console" );
    Console* Console::instance( nullptr );

    Console::Console( ConsolePtr console ): mConsole( move( console ) ), Wrapper( Wrapped_Console )
    {
      //
    }

    void Console::initialize( ConsolePtr console_, Local<v8::Context> context )
    {
      Isolate* isolate = Isolate::GetCurrent();
      HandleScope handleScope( isolate );
      v8::Context::Scope contextScope( context );

      Local<FunctionTemplate> tpl = FunctionTemplate::New( isolate );

      tpl->SetClassName( util::allocString( className.c_str() ) );
      tpl->InstanceTemplate()->SetInternalFieldCount( 1 );

      JS_TEMPLATE_SET( tpl, "print", jsPrint );
      JS_TEMPLATE_SET( tpl, "getVariable", jsGetVariable );
      JS_TEMPLATE_SET( tpl, "setVariable", jsSetVariable );
      JS_TEMPLATE_SET( tpl, "execute", jsExecute );

      instance = new Console( console_ );
      Local<v8::Object> object = tpl->GetFunction( context ).ToLocalChecked();
      instance->wrap( object );

      context->Global(); // ->Set( util::allocString( className.c_str() ), object );
      instance->ref();
    }

    void Console::shutdown()
    {
      if ( instance )
      {
        instance->unref();
        delete instance;
        instance = nullptr;
      }
    }

    void Console::jsPrint( const FunctionCallbackInfo<v8::Value>& args )
    {
      Console* ptr = unwrap( args.Holder() );
      HandleScope handleScope( args.GetIsolate() );

      utf8String msg;
      for ( int i = 0; i < args.Length(); i++ )
      {
        if ( i > 0 )
          msg.append( " " );
        v8::String::Utf8Value str( args.GetIsolate(), args[i] );
        if ( *str )
          msg.append( *str );
      }

      ptr->getConsole()->print( neko::Console::srcScripting, msg );
    }

    void Console::jsGetVariable( const FunctionCallbackInfo<v8::Value>& args )
    {
      v8::Isolate* isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      Console* console = unwrap( args.Holder() );

      if ( args.Length() != 1 || !args[0]->IsString() )
      {
        util::throwException( isolate,
          L"Syntax error: String Console.getVariable( String variable )" );
        return;
      }

      v8::String::Utf8Value variableName( args.GetIsolate(), args[0] );

      ConVar* variable = console->getConsole()->getVariable(
        (const char*)*variableName );

      if ( !variable )
      {
        util::throwException( isolate,
          L"Console.getVariable: No such console variable \"%s\"",
          (const char*)*variableName );
        return;
      }

      args.GetReturnValue().Set( util::allocString( variable->as_s(), isolate ) );
    }

    void Console::jsSetVariable( const FunctionCallbackInfo<v8::Value>& args )
    {
      v8::Isolate* isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      Console* console = unwrap( args.Holder() );

      if ( args.Length() != 2 || !args[0]->IsString() )
      {
        util::throwException( isolate,
          L"Syntax error: Console.setVariable( String variable, String value )" );
        return;
      }

      v8::String::Utf8Value variableName( isolate, args[0] );
      v8::String::Utf8Value variableValue( isolate, args[1] );

      ConVar* variable = console->getConsole()->getVariable( (const char*)*variableName );

      if ( !variable )
      {
        util::throwException( isolate,
          L"Console.setVariable: No such console variable \"%s\"",
          (const wchar_t*)*variableName );
        return;
      }

      variable->set( (const char*)*variableValue );
    }

    void Console::jsExecute( const FunctionCallbackInfo<v8::Value>& args )
    {
      v8::Isolate* isolate = args.GetIsolate();
      HandleScope handleScope( isolate );

      Console* console = unwrap( args.Holder() );

      if ( args.Length() != 1 || !args[0]->IsString() )
      {
        util::throwException( isolate,
          L"Syntax error: Console.execute( String commandLine )" );
        return;
      }

      v8::String::Utf8Value commandLine( isolate, args[0] );

      console->getConsole()->execute( (const char*)*commandLine );
    }

  }

}