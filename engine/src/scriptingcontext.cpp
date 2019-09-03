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

#include "v8pp/convert.hpp"

namespace neko {

  using v8::HandleScope;
  using v8::ObjectTemplate;
  using v8::FunctionTemplate;
  using v8::Isolate;
  using v8::Context;
  using v8::Local;
  using v8::Global;

  ScriptingContext::ScriptingContext( ScriptingPtr owner,
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
    HandleScope handleScope( isolate_ );

    auto globalObjectTemplate = ObjectTemplate::New( isolate_ );

    owner_->registerTemplateGlobals( isolate_, globalObjectTemplate );

    auto context = v8::Context::New( isolate_, nullptr, globalObjectTemplate );
    if ( context.IsEmpty() )
      NEKO_EXCEPT( "V8 context creation failed" );

    ctx_.Reset( isolate_, context );

    Context::Scope contextScope( context );

    owner_->registerContextGlobals( isolate_, ctx_ );
  }

  ScriptingContext::~ScriptingContext()
  {
    auto context = v8pp::to_local( isolate_, ctx_ );
    context->Exit();
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