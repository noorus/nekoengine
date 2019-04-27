#pragma once
#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "console.h"

namespace neko {

  //! \class Engine
  //! The main engine class that makes the world go round
  class Engine: public enable_shared_from_this<Engine> {
  public:
    //! Possible signal values interpreted by the engine's gameloop
    enum Signal {
      Signal_None = 0, //!< No signal.
      Signal_Stop //!< Engine stop signal.
    };
    //! Possible fatal errors
    enum FatalError {
      Fatal_Generic, //!< Non-specific fatal error case.
      Fatal_MemoryAllocation //!< Memory allocation failure.
    };
    //! Engine options structure
    struct Options {
      bool noAudio;
    };
  protected:
    ConsolePtr console_;
    GfxPtr gfx_;
    volatile Signal signal_;
  public:
    //! Constructor.
    Engine();
    //! Destructor.
    ~Engine();
    //! Raises a stop signal on the next cycle.
    void signalStop();
    //! Triggers a "graceful" quit in case of a fatal error.
    void triggerFatalError( FatalError error );
    //! Shutdown state for gfx restart.
    void operationSuspendVideo();
    //! Continue state after gfx restart.
    void operationContinueVideo();
    //! Shutdown state for audio restart.
    void operationSuspendAudio();
    //! Continue state after audio restart.
    void operationContinueAudio();
    //! Initializes the Engine.
    void initialize( const Options& options );
    //! Runs the Engine.
    void run();
    //! Shuts down the Engine and frees any resources it is using.
    void shutdown();
  };

}