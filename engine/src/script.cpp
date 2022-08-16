#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

# include "scripting.h"
# include "js_util.h"
# include "neko_platform.h"
# include "console.h"
# include "filesystem.h"

namespace neko {

  using v8::Context;
  using v8::EscapableHandleScope;
  using v8::FunctionCallbackInfo;
  using v8::FunctionTemplate;
  using v8::HandleScope;
  using v8::Isolate;
  using v8::Local;
  using v8::ObjectTemplate;
  using v8::Persistent;
  using v8::TryCatch;
  using v8::Value;

  Script::Script( ScriptingContext* globalCtx, const utf8String& name ):
    globalContext_( globalCtx ), name_( name ), status_( Status_Unknown )
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
      TextFileReader reader( Locator::fileSystem().openFile( Dir_Scripts, name_ ) );
      source = move( reader.readFullAssumeUtf8() );
    }

    v8::ScriptOrigin origin( isolate, js::util::allocString( name_ ) );

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
      globalContext_->console_->printf( Console::srcScripting, "● %s", *exception );
      globalContext_->console_->printf( Console::srcScripting, "In %s line %d:", *scriptName, lineNumber );
      globalContext_->console_->printf( Console::srcScripting, "→ %s", *sourceLine );
    }
  }

  js::V8Value Script::execute( v8::Global<v8::Context>& context_ )
  {
    auto isolate = globalContext_->isolate();
    Isolate::Scope isolateScope( isolate );
    HandleScope scope( isolate );
    EscapableHandleScope esc( isolate );

    auto context = Local<Context>::New( isolate, context_ );
    Context::Scope contextScope( context );

    retval_.Reset( isolate, v8::Null( isolate ) );

    if ( script_.IsEmpty() )
      return esc.Escape( js::V8Value::New( isolate, retval_ ) );

    globalContext_->console_->printf( Console::srcScripting, "Executing %s", name_.c_str() );

    TryCatch tryCatch( isolate );

    auto script = Local<v8::Script>::New( isolate, script_ );
    auto result = script->Run( context );

    if ( result.IsEmpty() )
    {
      if ( tryCatch.HasCaught() )
      {
        reportException( tryCatch );
        status_ = Status_RuntimeError;
        return esc.Escape( js::V8Value::New( isolate, retval_ ) );
      }
    }
    else
    {
      retval_.Reset( isolate, result.ToLocalChecked() );
    }

    status_ = Status_Executed;

    globalContext_->console_->printf( Console::srcScripting, "Executed %s", name_.c_str() );

    return esc.Escape( js::V8Value::New( isolate, retval_ ) );
  }

}

#endif