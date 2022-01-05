#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "forwards.h"
#include "scripting.h"
#include "neko_platform.h"
#include "engine.h"
#include "console.h"
#include "locator.h"
#include "memory.h"
#include "js_math.h"

namespace neko {

  using v8::HandleScope;
  using v8::ObjectTemplate;
  using v8::FunctionTemplate;
  using v8::Isolate;
  using v8::Context;
  using v8::Local;
  using v8::Global;

  ScriptingContext::ScriptingContext( Scripting* owner,
  v8::ArrayBuffer::Allocator* allocator, Isolate* isolate ):
  owner_( owner ), isolate_( isolate ), externalIsolate_( isolate ? true : false )
  {
    assert( owner_ );

    director_ = owner_->engine_->director();

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

    engine_ = owner->engine_;
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

  #define JS_SET_GLOBAL_FUNCTION( x ) global->Set( \
    js::util::allocStringConserve( #x, isolate_ ), \
    v8::FunctionTemplate::New( \
      isolate_, []( const v8::FunctionCallbackInfo<v8::Value>& args ) { \
        auto self = static_cast<ScriptingContext*>( args.Data().As<v8::External>()->Value() ); \
        self->js_##x( args );\
      }, \
      v8::External::New( isolate_, static_cast<void*>( this ) ) \
    ) )

  void ScriptingContext::registerTemplateGlobals( Local<ObjectTemplate>& global )
  {
    vec2Registry_.initialize( isolate_, global );
    vec3Registry_.initialize( isolate_, global );
    quatRegistry_.initialize( isolate_, global );
    meshRegistry_.initialize( isolate_, global );
    modelRegistry_.initialize( isolate_, global );
    textRegistry_.initialize( isolate_, global );

    JS_SET_GLOBAL_FUNCTION( include );
    JS_SET_GLOBAL_FUNCTION( require );
  }

  void ScriptingContext::js_include( const v8::FunctionCallbackInfo<v8::Value>& args )
  {
    // FIXME: This actually crashes and burns due to some sort of (handle?) heap fuckup

    HandleScope handleScope( isolate_ );

    if ( args.Length() != 1 || !args[0]->IsString() )
    {
      js::util::throwException( isolate_, "Expected file name as argument" );
      args.GetReturnValue().Set( false );
      return;
    }

    v8::String::Utf8Value filename( isolate_, args[0] );
    auto retval = addAndRunScript( *filename );

    args.GetReturnValue().Set( retval );
  }

  void ScriptingContext::js_require( const v8::FunctionCallbackInfo<v8::Value>& args )
  {
    // FIXME: This actually crashes and burns due to some sort of (handle?) heap fuckup

    HandleScope handleScope( isolate_ );

    if ( args.Length() != 1 || !args[0]->IsString() )
    {
      js::util::throwException( isolate_, "Expected file name as argument" );
      args.GetReturnValue().Set( false );
      return;
    }

    v8::String::Utf8Value filename( isolate_, args[0] );
    auto retval = requireScript( *filename );

    args.GetReturnValue().Set( retval );
  }

  void ScriptingContext::registerContextGlobals( Global<Context>& globalContext )
  {
    HandleScope handleScope( isolate_ );

    auto context = Local<Context>::New( isolate_, globalContext );

    jsConsole_ = js::Console::create( console_, isolate_, context->Global() );
    jsMath_ = js::Math::create( isolate_, context->Global() );
    jsGame_ = js::Game::create( isolate_, context->Global() );
  }

  js::V8Value ScriptingContext::addAndRunScript( const utf8String& filename )
  {
    HandleScope handleScope( isolate_ );

    if ( scripts_.find( filename ) == scripts_.end() )
    {
      auto script = make_shared<Script>( this, filename );
      scripts_[filename] = move( script );
    }

    auto& script = scripts_[filename];

    if ( !script->compiled() )
      if ( !script->compile( ctx_ ) )
        NEKO_EXCEPT( "Script compilation failed" );

    return script->execute( ctx_ );
  }

  js::V8Value ScriptingContext::requireScript( const utf8String& filename )
  {
    HandleScope handleScope( isolate_ );

    if ( scripts_.find( filename ) == scripts_.end() )
    {
      auto script = make_shared<Script>( this, filename );
      scripts_[filename] = move( script );
    }

    auto& script = scripts_[filename];

    if ( script->executed() )
      return script->getReturn( isolate_ );

    if ( !script->compiled() )
      if ( !script->compile( ctx_ ) )
        NEKO_EXCEPT( "Script compilation failed" );

    return script->execute( ctx_ );
  }

  void ScriptingContext::tick( GameTime tick, GameTime time )
  {
    isolate_->Enter();
    HandleScope handleScope( isolate_ );
    auto context = Local<Context>::New( isolate_, ctx_ );
    jsGame_->update( context, tick, time );

    // FIXME: no idea whether it makes more sense to run this in tick or process()
    renderSync().syncFromScripting();

    isolate_->Exit();
  }

  void ScriptingContext::process()
  {
    isolate_->PerformMicrotaskCheckpoint();

    v8::platform::PumpMessageLoop( owner_->platform_.get(), isolate_,
      v8::platform::MessageLoopBehavior::kDoNotWait );
  }

  ScriptingContext::~ScriptingContext()
  {
    textRegistry_.clear();
    modelRegistry_.clear();
    meshRegistry_.clear();
    quatRegistry_.clear();
    vec3Registry_.clear();
    vec2Registry_.clear();

    if ( isolate_ )
    {
      isolate_->IdleNotificationDeadline( owner_->platform_->MonotonicallyIncreasingTime() + 1.0 );
      platform::sleep( 1000 );
      isolate_->LowMemoryNotification();
    }

    jsGame_.reset();
    jsMath_.reset();
    jsConsole_.reset();

    ctx_.Reset();

    if ( isolate_ && !externalIsolate_ )
    {
      isolate_->ContextDisposedNotification();
      // None of these matter. It still leaks f*****g memory.
      isolate_->TerminateExecution();
      isolate_->Exit();
      isolate_->Dispose();
    }
  }

}

#endif