#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "js_console.h"
#include "js_math.h"
#include "js_mesh.h"
#include "js_model.h"
#include "js_game.h"
#include "director.h"

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

  }

  class Script {
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
    utf8String filename_;
    Status status_;
    v8::Persistent<v8::Script> script_;
    ScriptingContext* globalContext_;
    v8::Persistent<v8::Value> retval_;
  public:
    Script( ScriptingContext* globalCtx, const utf8String& name, const utf8String& filepath );
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

  class ScriptingContext {
  private:
    const bool externalIsolate_;
    v8::Isolate* isolate_;
    v8::Global<v8::Context> ctx_;
    Scripting* owner_;
    DirectorPtr director_;
    ScriptMap scripts_;
    void initialize();
  private:
    void registerTemplateGlobals( v8::Local<v8::ObjectTemplate>& global );
    void registerContextGlobals( v8::Global<v8::Context>& globalContext );
    void js_include( const v8::FunctionCallbackInfo<v8::Value>& args );
    void js_require( const v8::FunctionCallbackInfo<v8::Value>& args );
  protected:
    js::JSConsolePtr jsConsole_;
    js::JSMathPtr jsMath_;
    js::JSGamePtr jsGame_;
    js::DynamicObjectsRegistry<js::Vector2, vec2> vec2Registry_;
    js::DynamicObjectsRegistry<js::Vector3, vec3> vec3Registry_;
    js::DynamicObjectsRegistry<js::Quaternion, quaternion> quatRegistry_;
    js::DynamicObjectsRegistry<js::Mesh, JSMesh> meshRegistry_;
    js::DynamicObjectsRegistry<js::Model, js::JSModel> modelRegistry_;
  public:
    EnginePtr engine_;
    ConsolePtr console_;
    utf8String scriptDirectory_;
    ScriptingContext( Scripting* owner, v8::ArrayBuffer::Allocator* allocator, const utf8String& scriptDirectory, v8::Isolate* isolate = nullptr );
    void tick( GameTime tick, GameTime time );
    void process();
    ~ScriptingContext();
    inline v8::Isolate* isolate() const throw() { return isolate_; }
    inline v8::Global<v8::Context>& ctx() throw() { return ctx_; }
    inline js::DynamicObjectsRegistry<js::Vector2, vec2>& vec2reg() { return vec2Registry_; }
    inline js::DynamicObjectsRegistry<js::Vector3, vec3>& vec3reg() { return vec3Registry_; }
    inline js::DynamicObjectsRegistry<js::Quaternion, quaternion>& quatreg() { return quatRegistry_; }
    inline js::DynamicObjectsRegistry<js::Mesh, JSMesh>& meshreg() { return meshRegistry_; }
    inline js::DynamicObjectsRegistry<js::Model, js::JSModel>& modelreg() { return modelRegistry_; }
    inline RenderSyncContext& renderSync() { return director_->renderSync(); }
    js::V8Value addAndRunScript( const utf8String& filename );
    js::V8Value requireScript( const utf8String& filename );
  };

  class Scripting: public Subsystem, public v8::ArrayBuffer::Allocator {
    friend class ScriptingContext;
  protected:
    unique_ptr<v8::Platform> platform_;
    ScriptingContextPtr global_;
    wstring rootDirectory_;
    wstring dataDirectory_;
  private:
    //! v8::ArrayBuffer::Allocator implementation
    virtual void* Allocate( size_t length ) override;
    virtual void* AllocateUninitialized( size_t length ) override;
    virtual void Free( void* data, size_t length ) override;
  public:
    Scripting( EnginePtr engine );
    void initialize();
    void postInitialize();
    void shutdown();
    virtual void preUpdate( GameTime time ) override;
    virtual void tick( GameTime tick, GameTime time ) override;
    virtual void postUpdate( GameTime delta, GameTime tick ) override;
    virtual ~Scripting();
    inline ScriptingContextPtr getContext() throw() { return global_; }
  };

}

#endif