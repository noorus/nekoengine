#pragma once
#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "console.h"
#include "messaging.h"
#include "director.h"
#include "tankengine.h"

namespace neko {

  struct EngineInfo
  {
    uint32_t major;
    uint32_t minor;
    uint32_t build;
    utf8String profile;
    utf8String engineName;
    utf8String logName;
    utf8String compiled;
    utf8String compiler;
  };

  struct TankLibrary {
  public:
    HMODULE module_;
    tank::TankEngine* engine_;
    tank::fnTankInitialize pfnTankInitialize;
    tank::fnTankShutdown pfnTankShutdown;
    void load( tank::TankHost* host );
    void unload();
  };

  //! \class Engine
  //! The main engine class that makes the world go round
  class Engine: public enable_shared_from_this<Engine>, public Listener, public tank::TankHost {
  public:
    //! Possible signal values interpreted by the engine's gameloop
    enum Signal {
      Signal_None = 0, //!< No signal.
      Signal_Stop, //!< Engine stop signal.
      Signal_Restart
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
    //GfxPtr gfx_;
    ThreadedRendererPtr renderer_;
#ifndef NEKO_NO_SCRIPTING
    ScriptingPtr scripting_;
#endif
    ThreadedLoaderPtr loader_;
    FontManagerPtr fonts_;
    MessagingPtr messaging_;
    DirectorPtr director_;
    // InputPtr input_;
    EngineInfo info_;
    TankLibrary tanklib_;
    Environment env_;
  protected:
    struct State {
      bool focusLost;
      bool windowMove;
      bool steamOverlay;
      State(): focusLost( false ), windowMove( false ), steamOverlay( false ) {}
    } state_;
    platform::PerformanceClock clock_;
    GameTime time_;
    volatile Signal signal_;
    atomic<GameTime> rendererTime_;
    bool paused();
    void restart();
  protected:
    //! Message listener callback.
    void onMessage( const Message& msg ) override;
    //! TankEngine log callback.
    virtual void onDiscordDebugPrint( const utf8String& message );
    virtual void onDiscordUserImage( const uint64_t userId, size_t width, size_t height, const uint8_t* data );
    virtual void onSteamDebugPrint( const utf8String& message );
    virtual void onSteamOverlayToggle( bool enabled );
  public:
    inline const EngineInfo& info() const throw() { return info_; }
    inline ConsolePtr console() throw() { return console_; }
    inline Environment& env() throw() { return env_; }
    //inline GfxPtr gfx() throw() { return gfx_; }
    //inline InputPtr input() throw() { return input_; }
#ifndef NEKO_NO_SCRIPTING
    inline ScriptingPtr scripting() throw() { return scripting_; }
#endif
    inline GameTime time() const throw() { return time_; }
    inline ThreadedLoaderPtr loader() throw() { return loader_; }
    inline FontManagerPtr fonts() throw() { return fonts_; }
    inline MessagingPtr msgs() throw() { return messaging_; }
    inline DirectorPtr director() throw() { return director_; }
    inline atomic<GameTime>& renderTime() throw() { return rendererTime_; }
    const tank::GameInstallationState& installationInfo();
  public:
    //! Constructor.
    Engine( ConsolePtr console, const Environment& env );
    //! Destructor.
    ~Engine();
    //! Triggers a "graceful" quit in case of a fatal error.
    void triggerFatalError( FatalError error );
    //! Initializes the Engine.
    void initialize( const Options& options );
    //! Runs the Engine.
    void run();
    //! Shuts down the Engine and frees any resources it is using.
    void shutdown();
  };

}