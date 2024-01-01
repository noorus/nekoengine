#include "pch.h"
#ifndef NEKO_NO_SCRIPTING

# include "neko_types.h"
# include "forwards.h"
# include "scripting.h"
# include "neko_platform.h"
# include "engine.h"
# include "console.h"
# include "locator.h"
# include "memory.h"
# include "js_math.h"

namespace neko {

  using v8::Context;
  using v8::FunctionTemplate;
  using v8::Global;
  using v8::HandleScope;
  using v8::Isolate;
  using v8::Local;
  using v8::ObjectTemplate;

  ScriptingContext::ScriptingContext( Scripting* owner, v8::ArrayBuffer::Allocator* allocator, Isolate* isolate ):
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

  # define REG_INIT( name, type ) if ( ! ## name ## Registry_ ) \
    name ## Registry_ = make_shared<js:: ## type ## ::RegistryType>(); \
    name ## Registry_->initialize( isolate, global );

  # define REG_CLEAR( name ) name ## Registry_->clear();

  # define REG_RESET( name ) name ## Registry_.reset();

  void ScriptContextBaseRegistries::initializeRegistries( Isolate* isolate, Local<ObjectTemplate>& global )
  {
    REG_INIT( vec2, Vector2 )
    REG_INIT( vec3, Vector3 )
    REG_INIT( quaternion, Quaternion )
    REG_INIT( text, Text )
    REG_INIT( entity, Entity )

    // Entity components
    REG_INIT( transform_c, TransformComponent )
    REG_INIT( camera_c, CameraComponent )
  }

  void ScriptContextBaseRegistries::clearRegistries()
  {
    REG_CLEAR( vec2 )
    REG_CLEAR( vec3 )
    REG_CLEAR( quaternion )
    REG_CLEAR( text )
    REG_CLEAR( entity )

    // Entity components
    REG_CLEAR( transform_c )
    REG_CLEAR( camera_c )
  }

  void ScriptContextBaseRegistries::destroyRegistries()
  {
    REG_RESET( vec2 )
    REG_RESET( vec3 )
    REG_RESET( quaternion )
    REG_RESET( text )
    REG_RESET( entity )

    // Entity components
    REG_RESET( transform_c )
    REG_RESET( camera_c )
  }

  void ScriptingContext::registerTemplateGlobals( Local<ObjectTemplate>& global )
  {
    initializeRegistries( isolate_, global );
  }

  void ScriptingContext::registerContextGlobals( Global<Context>& globalContext )
  {
    HandleScope handleScope( isolate_ );

    auto context = Local<Context>::New( isolate_, globalContext );

    jsConsole_ = js::Console::create( console_, isolate_, context->Global() );
    jsMath_ = js::Math::create( isolate_, context->Global() );
    jsGame_ = js::Game::create( isolate_, context->Global() );
  }

  js::V8Value ScriptingContext::addAndRunScript( SManager& scene, const utf8String& filename )
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

  void ScriptingContext::tick( GameTime tick, GameTime time, SManager& scene )
  {
    sceneRuntimeDontTouch_ = &scene;

    isolate_->Enter();
    HandleScope handleScope( isolate_ );
    auto context = Local<Context>::New( isolate_, ctx_ );
    jsGame_->update( context, tick, time );

    // FIXME: no idea whether it makes more sense to run this in tick or process()
    renderSync().syncFromScripting();

    isolate_->Exit();

    for ( auto& [key, value] : transformComponents()->items() )
    {
      if ( !value )
        continue;

      // value->dirty is currently not ever set anyway, see next comment
      if ( !value->dirty() && !value->ent().scale->dirty() && !value->ent().rotate->dirty() && !value->ent().translate->dirty() )
        continue;

      auto& tn = scene.tn( value->ent().eid );

      if ( value->ent().scale->dirty() )
        tn.scale = value->ent().scale->v();
      if ( value->ent().rotate->dirty() )
        tn.rotate = value->ent().rotate->q();
      if ( value->ent().translate->dirty() )
        tn.translate = value->ent().translate->v();

      value->ent().scale->markClean();
      value->ent().rotate->markClean();
      value->ent().translate->markClean();

      // we don't actually care about the component's dirty flag because the members' dirty state
      // does not propagate there in the first place, but let's clear it for completeness' sake
      value->markClean();

      scene.reg().emplace_or_replace<c::dirty_transform>( value->ent().eid );
    }

    sceneRuntimeDontTouch_ = nullptr;
  }

  void ScriptingContext::process( SManager& scene )
  {
    sceneRuntimeDontTouch_ = &scene;

    isolate_->PerformMicrotaskCheckpoint();
    v8::platform::PumpMessageLoop( owner_->platform_.get(), isolate_, v8::platform::MessageLoopBehavior::kDoNotWait );

    sceneRuntimeDontTouch_ = nullptr;
  }

  ScriptingContext::~ScriptingContext()
  {
    clearRegistries();
    destroyRegistries();

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