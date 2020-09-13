#pragma once
#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "neko_platform.h"

namespace neko {

  enum MessageCode {
    M_Window_LostFocus,
    M_Window_GainedFocus,
    M_Window_EnterMove,
    M_Window_ExitMove,
    M_Window_Close
  };

  struct Message {
    MessageCode code;
    vector<void*> arguments;
  };

  using MessageVector = vector<Message>;

  class Messaging;

  class Listener {
    friend class Messaging;
  private:
    virtual void onMessage( const Message& msg ) = 0;
  };

  class Messaging: public Subsystem {
  public:
    struct ListenEntry {
      size_t groups_;
      Listener* callback_;
      ListenEntry( size_t groups, Listener* callback ): groups_( groups ), callback_( callback ) {}
    };
    using Listeners = vector<ListenEntry>;
  protected:
    Listeners listeners_;
    MessageVector messages_;
    platform::RWLock lock_;
  public:
    Messaging( EnginePtr engine );
    void send( MessageCode code, int numargs = 0, ... );
    void processEvents(); //!< Process vital events regardless of pause state (window events etc)
    void preUpdate( GameTime time ) override;
    void tick( GameTime tick, GameTime time ) override;
    void postUpdate( GameTime delta, GameTime tick ) override;
    void listen( Listener* callback );
    void remove( Listener* callback );
    virtual ~Messaging();
  };

}