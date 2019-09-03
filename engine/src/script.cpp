#include "stdafx.h"
#ifndef NEKO_NO_SCRIPTING

#include "scripting.h"
#include "js_util.h"
#include "neko_platform.h"
#include "console.h"

namespace neko {

  using v8::Isolate;
  using v8::Persistent;
  using v8::Local;
  using v8::Context;
  using v8::TryCatch;
  using v8::HandleScope;
  using v8::FunctionCallbackInfo;
  using v8::ObjectTemplate;
  using v8::FunctionTemplate;
  using v8::Value;

  Script::Script( ScriptingContextPtr globalCtx, utf8String name ):
    globalContext_( move( globalCtx ) ),
    name_( name ), filename_( name )
  {
    //
  }

  bool Script::compile( v8::Global<v8::Context>& context_ )
  {
    auto isolate = globalContext_->isolate();
    Isolate::Scope isolateScope( isolate );
    HandleScope handleScope( isolate );

    globalContext_->console_->printf( Console::srcScripting, "Compiling %s", name_.c_str() );

    status_ = Status_Unknown;

    auto context = Local<Context>::New( isolate, context_ );
    Context::Scope contextScope( context );

    TryCatch tryCatch( isolate );

    utf8String source;
    {
      TextFileReader reader( filename_ );
      source = move( reader.readFullAssumeUtf8() );
    }

    v8::ScriptOrigin origin( js::util::allocString( name_ ) );

    auto script = v8::Script::Compile( context, js::util::allocString( source ) );
    if ( script.IsEmpty() )
    {
      if ( tryCatch.HasCaught() )
        reportException( tryCatch );
      status_ = Status_CompileError;
      return false;
    }

    script_.Reset( isolate, script.ToLocalChecked() );
    status_ = Status_Compiled;

    globalContext_->console_->printf( Console::srcScripting, "%s compiled succesfully!", name_.c_str() );

    return true;
  }

  void Script::reportException( const v8::TryCatch& tryCatch )
  {
    auto isolate = globalContext_->isolate();
    Isolate::Scope isolateScope( isolate );
    HandleScope handleScope( isolate );

    v8::String::Utf8Value exception( isolate, tryCatch.Exception() );
    v8::Local<v8::Message> message = tryCatch.Message();

    if ( message.IsEmpty() )
    {
      globalContext_->console_->printf( Console::srcScripting, "V8 Exception: %s", *exception );
    }
    else
    {
      v8::Local<v8::Context> context( isolate->GetCurrentContext() );

      v8::String::Utf8Value scriptName( isolate, message->GetScriptOrigin().ResourceName() );
      v8::String::Utf8Value sourceLine( isolate, message->GetSourceLine( context ).ToLocalChecked() );
      auto lineNumber = message->GetLineNumber( context ).FromJust();

      globalContext_->console_->printf( Console::srcScripting, "V8 Script exception!" );
      globalContext_->console_->printf( Console::srcScripting, "●   %s", *exception );
      globalContext_->console_->printf( Console::srcScripting, "In %s line %d:", *scriptName, lineNumber );
      globalContext_->console_->printf( Console::srcScripting, "→   %s", *sourceLine );
    }
  }

  bool Script::execute( v8::Global<v8::Context>& context_ )
  {
    auto isolate = globalContext_->isolate();
    Isolate::Scope isolateScope( isolate );
    HandleScope handleScope( isolate );

    if ( script_.IsEmpty() )
      return false;

    globalContext_->console_->printf( Console::srcScripting, "Running %s", name_.c_str() );

    auto context = Local<Context>::New( isolate, context_ );
    Context::Scope contextScope( context );

    TryCatch tryCatch( isolate );

    auto script = Local<v8::Script>::New( isolate, script_ );
    auto result = script->Run( context );

    if ( result.IsEmpty() )
    {
      if ( tryCatch.HasCaught() )
        reportException( tryCatch );
      status_ = Status_RuntimeError;
      return false;
    }

    globalContext_->console_->printf( Console::srcScripting, "Executed %s fine!", name_.c_str() );

    return true;
  }

}

#endif // !NEKO_NO_SCRIPTING