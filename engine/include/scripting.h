#pragma once
#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"

namespace neko {

  class Scripting: public Subsystem {
  protected:
    v8::Isolate* isolate_;
    v8::Global<v8::Context> context_;
    unique_ptr<v8::Platform> platform_;
    unique_ptr<v8::ArrayBuffer::Allocator> allocator_;
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