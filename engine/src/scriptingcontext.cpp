#include "stdafx.h"
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "forwards.h"
#include "scripting.h"
#include "neko_platform.h"
#include "engine.h"
#include "console.h"
#include "locator.h"
#include "memory.h"

namespace neko {

  using v8::HandleScope;
  using v8::ObjectTemplate;
  using v8::FunctionTemplate;
  using v8::Isolate;
  using v8::Context;
  using v8::Local;
  using v8::Global;

  ScriptingContext::ScriptingContext( Scripting* owner,
    v8::ArrayBuffer::Allocator* allocator,
    Isolate* isolate ):
    owner_( owner ), isolate_( isolate ), externalIsolate_( isolate ? true : false )
  {
    assert( owner_ );

    if ( !isolate_ )
    {
      Isolate::CreateParams params;
      params.array_buffer_allocator = allocator;
      isolate_ = Isolate::New( params );
      if ( !isolate_ )
        NEKO_EXCEPT( "V8 isolate creation failed" );
      isolate_->Enter();
    }

    console_ = owner_->engine_->console();

    initialize();
  }

  void ScriptingContext::initialize()
  {
    Isolate::Scope isolateScope( isolate_ );
    HandleScope handleScope( isolate_ );

    auto globalObjectTemplate = ObjectTemplate::New( isolate_ );

    registerTemplateGlobals( globalObjectTemplate );

    auto context = v8::Context::New( isolate_, nullptr, globalObjectTemplate );
    if ( context.IsEmpty() )
      NEKO_EXCEPT( "V8 context creation failed" );

    ctx_.Reset( isolate_, context );

    {
      Context::Scope contextScope( context );
      registerContextGlobals( ctx_ );
    }
  }

  void ScriptingContext::registerTemplateGlobals( v8::Local<v8::ObjectTemplate>& global )
  {
    // global->Set( js::util::allocString( "fuckyou" ),  )
  }

  void ScriptingContext::registerContextGlobals( v8::Global<v8::Context>& globalContext )
  {
    v8::HandleScope handleScope( isolate_ );

    auto context = v8::Local<v8::Context>::New( isolate_, globalContext );

    jsConsole_ = js::Console::create( console_, isolate_, context->Global() );
  }

  void ScriptingContext::tick()
  {
    Isolate::Scope isolateScope( isolate_ );
    v8::HandleScope handleScope( isolate_ );

    auto context = v8::Local<v8::Context>::New( isolate_, ctx_ );
    Context::Scope contextScope( context );

    v8::platform::PumpMessageLoop( owner_->platform_.get(), isolate_, v8::platform::MessageLoopBehavior::kDoNotWait );
  }

  ScriptingContext::~ScriptingContext()
  {
    jsConsole_.reset();

    ctx_.Reset();

    if ( isolate_ && !externalIsolate_ )
    {
      isolate_->Exit();
      isolate_->ContextDisposedNotification();
      // None of these matter. It still leaks f*****g memory.
      /*isolate_->IdleNotificationDeadline( platform_->MonotonicallyIncreasingTime() + 1.0 );
      isolate_->LowMemoryNotification();
      Sleep( 2000 );*/
      isolate_->TerminateExecution();
      isolate_->Dispose();
    }
  }

}

#endif // !NEKO_NO_SCRIPTING