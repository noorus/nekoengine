#pragma once
#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"

namespace neko {

  class Scripting: public Subsystem, public v8::ArrayBuffer::Allocator {
  protected:
    v8::Isolate* isolate_;
    v8::Global<v8::Context> context_;
    unique_ptr<v8::Platform> platform_;
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
    inline v8::Isolate* getIsolate() throw() { return isolate_; }
    inline v8::Global<v8::Context>& getContext() throw() { return context_; }
  };

}