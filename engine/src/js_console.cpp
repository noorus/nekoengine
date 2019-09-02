#include "stdafx.h"
#include "js_console.h"
#include "js_util.h"
#include "console.h"
#include "v8pp/convert.hpp"

namespace neko {

  namespace js {

    string Console::className( "Console" );
    Console* Console::instance( nullptr );

    v8::Global<v8::FunctionTemplate> Console::constructor;

    Console::Console( ConsolePtr console ): mConsole( move( console ) )
    {
      //
    }

    void Console::initialize( ConsolePtr console_, Isolate* isolate, Local<v8::Context> context )
    {
      HandleScope handleScope( isolate );

      Local<FunctionTemplate> tpl = FunctionTemplate::New( isolate );

      tpl->SetClassName( util::allocString( className.c_str() ) );
      tpl->InstanceTemplate()->SetInternalFieldCount( 1 );

      JS_TEMPLATE_SET( tpl, "print", jsPrint );
      JS_TEMPLATE_SET( tpl, "getVariable", jsGetVariable );
      JS_TEMPLATE_SET( tpl, "setVariable", jsSetVariable );
      JS_TEMPLATE_SET( tpl, "execute", jsExecute );

      auto obj = v8pp::to_local( isolate, constructor )
        ->GetFunction( context ).ToLocalChecked()
        ->NewInstance( context ).ToLocalChecked();

      instance = new Console( console_ );
      obj->SetAlignedPointerInInternalField( 0, console_.get() );

      instance->jsHandle_.Reset( isolate, obj );
      instance->makeWeak();

      context->Global()->Set( util::allocString( className.c_str() ), obj );
      instance->ref();
    }

    void Console::makeWeak()
    {
      instance->jsHandle_.SetWeak( instance, []( v8::WeakCallbackInfo<Console> const& data )
      {
        auto object = static_cast<Console*>( data.GetInternalField( 0 ) );
        // what do?
      }, v8::WeakCallbackType::kInternalFields );
    }

    void Console::ref()
    {
      assert( !persistent().IsEmpty() );
      persistent().ClearWeak();
      jsReferences_++;
    }

    void Console::unref()
    {
      assert( !persistent().IsEmpty() );
      assert( !persistent().IsWeak() );
      assert( jsReferences_ > 0 );
      if ( --jsReferences_ == 0 )
        makeWeak();
    }

    template <class T>
    static inline T* unwrap( Isolate* isolate, v8::Local<v8::Value> value )
    {
      v8::HandleScope scope( isolate );
      while ( value->IsObject() )
      {
        v8::Local<v8::Object> obj = value.As<v8::Object>();
        if ( obj->InternalFieldCount() == 1 )
        {
          auto ptr = obj->GetAlignedPointerFromInternalField( 0 );
          return static_cast<T*>( ptr );
        }
        value = obj->GetPrototype();
      }
      return nullptr;
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
      auto isolate = args.GetIsolate();
      auto ptr = unwrap<Console>( isolate, args.Holder() );
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

      ptr->getConsole()->print( neko::Console::srcScripting, msg );
    }

    void Console::jsGetVariable( const FunctionCallbackInfo<v8::Value>& args )
    {
      auto isolate = args.GetIsolate();
      auto ptr = unwrap<Console>( isolate, args.Holder() );
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 || !args[0]->IsString() )
      {
        util::throwException( isolate,
          L"Syntax error: String Console.getVariable( String variable )" );
        return;
      }

      v8::String::Utf8Value variableName( args.GetIsolate(), args[0] );

      ConVar* variable = ptr->getConsole()->getVariable(
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
      auto isolate = args.GetIsolate();
      auto ptr = unwrap<Console>( isolate, args.Holder() );
      HandleScope handleScope( isolate );

      if ( args.Length() != 2 || !args[0]->IsString() )
      {
        util::throwException( isolate,
          L"Syntax error: Console.setVariable( String variable, String value )" );
        return;
      }

      v8::String::Utf8Value variableName( isolate, args[0] );
      v8::String::Utf8Value variableValue( isolate, args[1] );

      ConVar* variable = ptr->getConsole()->getVariable( (const char*)*variableName );

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
      auto isolate = args.GetIsolate();
      auto ptr = unwrap<Console>( isolate, args.Holder() );
      HandleScope handleScope( isolate );

      if ( args.Length() != 1 || !args[0]->IsString() )
      {
        util::throwException( isolate,
          L"Syntax error: Console.execute( String commandLine )" );
        return;
      }

      v8::String::Utf8Value commandLine( isolate, args[0] );

      ptr->getConsole()->execute( (const char*)*commandLine );
    }

  }

}