#include "pch.h"
#include "subsystem.h"
#include "neko_exception.h"

namespace neko {

  Subsystem::Subsystem( EnginePtr engine ): engine_( move( engine ) )
  {
  }

  void Subsystem::preUpdate( GameTime time )
  {
    NEKO_EXCEPT( "Called preUpdate on a Subsystem that does not implement it" );
  }

  void Subsystem::tick( GameTime tick, GameTime time )
  {
    NEKO_EXCEPT( "Called tick on a Subsystem that does not implement it" );
  }

  void Subsystem::postUpdate( GameTime delta, GameTime tick )
  {
    NEKO_EXCEPT( "Called postUpdate on a Subsystem that does not implement it" );
  }

  Subsystem::~Subsystem()
  {
    //
  }

}