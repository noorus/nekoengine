#include "pch.h"
#include "neko_exception.h"
#include "engine.h"
#include "messaging.h"
#include "console.h"

namespace neko {

  Messaging::Messaging( EnginePtr engine ): Subsystem( move( engine ) )
  {
    //
  }

  void Messaging::send( MessageCode code, int numargs, ... )
  {
    Message msg;
    msg.code = code;

    va_list va_alist;
    va_start( va_alist, numargs );
    for ( size_t i = 0; i < numargs; i++ )
    {
      void* arg = va_arg( va_alist, void* );
      msg.arguments.push_back( arg );
    }
    va_end( va_alist );

    lock_.lock();
    messages_.push_back( msg );
    lock_.unlock();
  }

  void Messaging::preUpdate( GameTime time )
  {
    //
  }

  void Messaging::processEvents()
  {
    MessageVector messagesBuffer;
    messagesBuffer.swap( messages_ );

    for ( auto& msg : messagesBuffer )
    {
      for ( auto& listener : listeners_ )
        listener.callback_->onMessage( msg );
    }
  }

  void Messaging::tick( GameTime tick, GameTime time )
  {
    //
  }

  void Messaging::postUpdate( GameTime delta, GameTime tick )
  {
    //
  }

  void Messaging::listen( Listener* callback )
  {
    remove( callback );

    ListenEntry entry( 0, callback );
    listeners_.push_back( entry );
  }

  void Messaging::remove( Listener* callback )
  {
    for ( auto it = listeners_.begin(); it != listeners_.end(); )
      if ( ( *it ).callback_ == callback ) { it = listeners_.erase( it ); } else { it++; }
  }

  Messaging::~Messaging()
  {
    //
  }

}