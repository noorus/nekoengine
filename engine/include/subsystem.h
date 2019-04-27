#pragma once
#include "neko_types.h"
#include "forwards.h"

namespace neko {

  class Subsystem: public noncopyable {
  protected:
    EnginePtr engine_;
  public:
    Subsystem( EnginePtr engine );
    virtual void preUpdate( GameTime time );
    virtual void tick( GameTime tick, GameTime time );
    virtual void postUpdate( GameTime delta, GameTime tick );
    virtual ~Subsystem();
  };

}