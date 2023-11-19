#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "js_console.h"
#include "js_math.h"
#include "js_game.h"
#include "js_text.h"
#include "js_entity.h"
#include "js_component.h"
#include "components.h"
#include "director.h"
#include "js_types.h"
#include "js_dynamicobject.h"
#include "js_dynamicregistry.h"
#include "js_util.h"

namespace neko {

  class ScriptingContext;
  using ScriptingContextPtr = shared_ptr<ScriptingContext>;

  namespace js {

    enum IsolateDataSlot {
      IsolateData_ScriptingContext = 0
    };

    inline ScriptingContext* getScriptContext( Isolate* isolate )
    {
      return static_cast<ScriptingContext*>( isolate->GetData( IsolateData_ScriptingContext ) );
    }

    inline ScriptingContext* scriptContext( Isolate* isolate )
    {
      return static_cast<ScriptingContext*>( isolate->GetData( IsolateData_ScriptingContext ) );
    }

  }

  class Script: public nocopy {
  public:
    enum Status {
      Status_Unknown,
      Status_Compiled,
      Status_CompileError,
      Status_RuntimeError,
      Status_Executed
    };
  private:
    utf8String name_;
    Status status_;
    v8::Persistent<v8::Script> script_;
    ScriptingContext* globalContext_;
    v8::Persistent<v8::Value> retval_;
  public:
    Script( ScriptingContext* globalCtx, const utf8String& name );
    inline bool compiled() const { return ( status_ == Status_Compiled ); }
    inline bool executed() const { return ( status_ == Status_Executed ); }
    bool compile( v8::Global<v8::Context>& context );
    void reportException( const v8::TryCatch& tryCatch );
    js::V8Value execute( v8::Global<v8::Context>& context_ );
    inline js::V8Value getReturn( v8::Isolate* isolate ) const
    {
      assert( status_ == Status_Executed );
      return js::V8Value::New( isolate, retval_ );
    }
  };

  using ScriptPtr = shared_ptr<Script>;
  using ScriptMap = map<utf8String, ScriptPtr>;

  using V8FunctionCallback = void(*)( const v8::FunctionCallbackInfo<v8::Value>& args );

# define SCRIPTCONTEXTBASE_DECLARE_REGISTRY( Type, Lowercase ) \
protected:                                                     \
  Type::RegistryPtrType Lowercase##Registry_;                  \
public:                                                        \
  template <typename T>                                        \
  T::RegistryPtrType registry( type<Type> )                    \
  {                                                            \
   return Lowercase##Registry_;                                \
  }

  class ScriptContextBaseRegistries {
  private: // Declare registries here using the macro above
    SCRIPTCONTEXTBASE_DECLARE_REGISTRY( js::Vector2, vec2 )
    SCRIPTCONTEXTBASE_DECLARE_REGISTRY( js::Vector3, vec3 )
    SCRIPTCONTEXTBASE_DECLARE_REGISTRY( js::Quaternion, quaternion )
    SCRIPTCONTEXTBASE_DECLARE_REGISTRY( js::Text, text )
    SCRIPTCONTEXTBASE_DECLARE_REGISTRY( js::Entity, entity )
    SCRIPTCONTEXTBASE_DECLARE_REGISTRY( js::TransformComponent, transform_c )
    SCRIPTCONTEXTBASE_DECLARE_REGISTRY( js::CameraComponent, camera_c )
  protected:
    void initializeRegistries( js::Isolate* isolate, js::Local<js::ObjectTemplate>& global );
    void clearRegistries();
    void destroyRegistries();
  };

  class ScriptingContext: public nocopy, public ScriptContextBaseRegistries {
  private:
    const bool externalIsolate_;
    v8::Isolate* isolate_;
    v8::Global<v8::Context> ctx_;
    Scripting* owner_;
    DirectorPtr director_;
    ScriptMap scripts_;
    SManager* sceneRuntimeDontTouch_ = nullptr;
    void initialize();
  private:
    void registerTemplateGlobals( v8::Local<v8::ObjectTemplate>& global );
    void registerContextGlobals( v8::Global<v8::Context>& globalContext );
  protected:
    js::JSConsolePtr jsConsole_;
    js::JSMathPtr jsMath_;
    js::JSGamePtr jsGame_;
  public:
    EnginePtr engine_;
    ConsolePtr console_;
    ScriptingContext( Scripting* owner, v8::ArrayBuffer::Allocator* allocator, v8::Isolate* isolate = nullptr );
    void tick( GameTime tick, GameTime time, SManager& scene );
    void process( SManager& scene );
    ~ScriptingContext();
    inline v8::Isolate* isolate() const noexcept { return isolate_; }
    inline v8::Global<v8::Context>& ctx() noexcept { return ctx_; }
    inline js::Vector2::RegistryPtrType vec2reg() { return registry<js::Vector2>( type<js::Vector2>() ); }
    inline js::Vector3::RegistryPtrType vec3reg() { return registry<js::Vector3>( type<js::Vector3>() ); }
    inline js::Quaternion::RegistryPtrType quatreg() { return registry<js::Quaternion>( type<js::Quaternion>() ); }
    inline js::Entity::RegistryPtrType entreg() { return registry<js::Entity>( type<js::Entity>() ); }
    inline js::Text::RegistryPtrType textreg() { return registry<js::Text>( type<js::Text>() ); }
    inline js::TransformComponent::RegistryPtrType transformComponents() { return registry<js::TransformComponent>( type<js::TransformComponent>() ); }
    inline js::CameraComponent::RegistryPtrType cameraComponents() { return registry<js::CameraComponent>( type<js::CameraComponent>() ); }
    inline RenderSyncContext& renderSync() { return director_->renderSync(); }
    inline SManager& scene()
    {
      assert( sceneRuntimeDontTouch_ );
      return *sceneRuntimeDontTouch_;
    }
    js::V8Value addAndRunScript( SManager& scene, const utf8String& filename );
    js::V8Value requireScript( const utf8String& filename );
  };

  class Scripting: public Subsystem<Scripting, srcScripting>, public v8::ArrayBuffer::Allocator {
    friend class ScriptingContext;
  protected:
    unique_ptr<v8::Platform> platform_;
    ScriptingContextPtr global_;
    wstring rootDirectory_;
    wstring dataDirectory_;
  private:
    //! v8::ArrayBuffer::Allocator implementation
    void* Allocate( size_t length ) override;
    void* AllocateUninitialized( size_t length ) override;
    void Free( void* data, size_t length ) override;
  public:
    Scripting( EnginePtr engine );
    void initialize();
    void postInitialize();
    void shutdown();
    void preUpdate( GameTime time ) override;
    void tick( GameTime tick, GameTime time ) override;
    void postUpdate( GameTime delta, GameTime tick ) override;
    virtual ~Scripting();
    inline ScriptingContextPtr getContext() noexcept { return global_; }
  };

}

#endif