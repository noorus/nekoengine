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
  using v8::Isolate;
  using v8::Context;
  using v8::Local;
  using v8::Global;

  ScriptingContext::ScriptingContext(
    v8::ArrayBuffer::Allocator* allocator,
    Isolate* isolate ):
    isolate_( isolate ), externalIsolate_( isolate ? true : false )
  {
    if ( !isolate_ )
    {
      Isolate::CreateParams params;
      params.array_buffer_allocator = allocator;
      isolate_ = Isolate::New( params );
      if ( !isolate_ )
        NEKO_EXCEPT( "V8 isolate creation failed" );
      isolate_->Enter();
    }

    HandleScope scope( isolate_ );

    auto global = ObjectTemplate::New( isolate_ );

    auto context = v8::Context::New( isolate_, nullptr, global );
    if ( context.IsEmpty() )
      NEKO_EXCEPT( "V8 context creation failed" );

    context->Enter();
    ctx_.Reset( isolate_, context );
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