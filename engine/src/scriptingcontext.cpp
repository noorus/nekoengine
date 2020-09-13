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
      params.allow_atomics_wait = true;
      isolate_ = Isolate::New( params );
      if ( !isolate_ )
        NEKO_EXCEPT( "V8 isolate creation failed" );
      isolate_->Enter();
    }

    isolate_->SetData( js::IsolateData_ScriptingContext, this );
    isolate_->SetMicrotasksPolicy( v8::MicrotasksPolicy::kExplicit );

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
    vec2Registry_.initialize( isolate_, global );
    vec3Registry_.initialize( isolate_, global );
  }

  void ScriptingContext::registerContextGlobals( v8::Global<v8::Context>& globalContext )
  {
    v8::HandleScope handleScope( isolate_ );

    auto context = v8::Local<v8::Context>::New( isolate_, globalContext );

    jsConsole_ = js::Console::create( console_, isolate_, context->Global() );
    jsMath_ = js::Math::create( isolate_, context->Global() );
    jsGame_ = js::Game::create( isolate_, context->Global() );
  }

  void ScriptingContext::tick( GameTime tick, GameTime time )
  {
    isolate_->Enter();
    v8::HandleScope handleScope( isolate_ );
    auto context = v8::Local<v8::Context>::New( isolate_, ctx_ );
    jsGame_->update( context, tick, time );
    isolate_->Exit();
  }

  void ScriptingContext::process()
  {
    isolate_->RunMicrotasks();
    v8::platform::PumpMessageLoop( owner_->platform_.get(), isolate_, v8::platform::MessageLoopBehavior::kDoNotWait );
  }

  ScriptingContext::~ScriptingContext()
  {
    vec2Registry_.clear();
    vec3Registry_.clear();
    jsGame_.reset();
    jsConsole_.reset();
    jsMath_.reset();

    ctx_.Reset();

    if ( isolate_ && !externalIsolate_ )
    {
      isolate_->Exit();
      isolate_->ContextDisposedNotification();
      // None of these matter. It still leaks f*****g memory.
      isolate_->IdleNotificationDeadline( owner_->platform_->MonotonicallyIncreasingTime() + 1.0 );
      isolate_->LowMemoryNotification();
      isolate_->TerminateExecution();
      isolate_->Dispose();
    }
  }

}

#endif // !NEKO_NO_SCRIPTING