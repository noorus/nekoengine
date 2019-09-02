#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"

namespace neko {

  class ScriptingContext;
  using ScriptingContextPtr = shared_ptr<ScriptingContext>;

  class Script {
  public:
    enum Status {
      Status_Unknown,
      Status_Compiled,
      Status_CompileError,
      Status_RuntimeError
    };
  private:
    utf8String name_;
    utf8String filename_;
    Status status_;
    v8::Persistent<v8::Script> script_;
    ScriptingContextPtr globalContext_;
  public:
    Script( ScriptingContextPtr globalCtx, utf8String name );
    bool compile( v8::Global<v8::Context>& context );
    void reportException( const v8::TryCatch& tryCatch );
    bool execute( v8::Global<v8::Context>& context_ );
  };

  class ScriptingContext {
  public:
    utf8String scriptDirectory_;
    v8::Isolate* isolate_;
    v8::Global<v8::Context> ctx_;
    const bool externalIsolate_;
    ConsolePtr console_;
    ScriptingContext( v8::ArrayBuffer::Allocator* allocator, v8::Isolate* isolate = nullptr );
    ~ScriptingContext();
    inline v8::Isolate* isolate() const throw() { return isolate_; }
  };

  class Scripting: public Subsystem, public v8::ArrayBuffer::Allocator {
  protected:
    unique_ptr<v8::Platform> platform_;
    ScriptingContextPtr global_;
    utf8String rootDirectory_;
    utf8String dataDirectory_;
  private:
    //! v8::ArrayBuffer::Allocator implementation
    virtual void* Allocate( size_t length ) override;
    virtual void* AllocateUninitialized( size_t length ) override;
    virtual void Free( void* data, size_t length ) override;
  public:
    Scripting( EnginePtr engine );
    void initialize();
    void shutdown();
    virtual void preUpdate( GameTime time ) override;
    virtual void tick( GameTime tick, GameTime time ) override;
    virtual void postUpdate( GameTime delta, GameTime tick ) override;
    virtual ~Scripting();
    inline ScriptingContextPtr getContext() throw() { return global_; }
  };

}

#endif // !NEKO_NO_SCRIPTING