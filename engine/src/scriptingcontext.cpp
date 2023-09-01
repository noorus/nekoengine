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

  void ScriptContextBaseRegistries::initializeRegistries( Isolate* isolate, Local<ObjectTemplate>& global )
  {
    if ( !vec2Registry_ )
      vec2Registry_ = make_shared<js::Vector2::RegistryType>();
    vec2Registry_->initialize( isolate, global );

    if ( !vec3Registry_ )
      vec3Registry_ = make_shared<js::Vector3::RegistryType>();
    vec3Registry_->initialize( isolate, global );

    if ( !quaternionRegistry_ )
      quaternionRegistry_ = make_shared<js::Quaternion::RegistryType>();
    quaternionRegistry_->initialize( isolate, global );

    if ( !meshRegistry_ )
      meshRegistry_ = make_shared<js::Mesh::RegistryType>();
    meshRegistry_->initialize( isolate, global );

    if ( !modelRegistry_ )
      modelRegistry_ = make_shared<js::Model::RegistryType>();
    modelRegistry_->initialize( isolate, global );

    if ( !textRegistry_ )
      textRegistry_ = make_shared<js::Text::RegistryType>();
    textRegistry_->initialize( isolate, global );

    if ( !entityRegistry_ )
      entityRegistry_ = make_shared<js::Entity::RegistryType>();
    entityRegistry_->initialize( isolate, global );

    if ( !componentRegistry_ )
      componentRegistry_ = make_shared<js::TransformComponent::RegistryType>();
    componentRegistry_->initialize( isolate, global );
  }

  void ScriptContextBaseRegistries::clearRegistries()
  {
    vec2Registry_->clear();
    vec3Registry_->clear();
    quaternionRegistry_->clear();
    meshRegistry_->clear();
    modelRegistry_->clear();
    textRegistry_->clear();
    entityRegistry_->clear();
    componentRegistry_->clear();
  }

  void ScriptContextBaseRegistries::destroyRegistries()
  {
    vec2Registry_.reset();
    vec3Registry_.reset();
    quaternionRegistry_.reset();
    meshRegistry_.reset();
    modelRegistry_.reset();
    textRegistry_.reset();
    entityRegistry_.reset();
    componentRegistry_.reset();
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

    for ( auto& [key, value] : compreg()->items() )
    {
      if ( !value || !value->dirty() )
        continue;
      if ( !value->dirty() && !value->ent().scale->dirty() && !value->ent().rotate->dirty() && !value->ent().translate->dirty() )
        continue;
      auto& tn = scene.tn( value->ent().eid );
      tn.scale = value->ent().scale->v();
      tn.rotate = value->ent().rotate->q();
      tn.translate = value->ent().translate->v();
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