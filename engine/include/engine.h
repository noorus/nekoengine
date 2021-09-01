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

  struct Stats {
    atomic<int> i_launches = 0;
    atomic<float> f_timeWasted = 0.0f;
  };

  //! \class Engine
  //! The main engine class that makes the world go round
  class Engine: public enable_shared_from_this<Engine>, public nocopy, public Listener, public tank::TankHost {
  public:
    //! Possible signal values interpreted by the engine's gameloop
    enum Signal {
      Signal_None = 0, //!< No signal.
      Signal_Stop, //!< Engine stop signal.
      Signal_Restart
    };
    //! Possible fatal errors
    enum class FatalError {
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
    Stats stats_;
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
    void onDiscordDebugPrint( const utf8String& message ) override;
    void onDiscordUserImage( const tank::DcSnowflake id, tank::Image& image ) override;
    void onSteamUserImage( const tank::SteamSnowflake id, tank::Image& image ) override;
    void onSteamDebugPrint( const utf8String& message ) override;
    void onSteamOverlayToggle( bool enabled ) override;
    void onSteamStatsUpdated( tank::StateUpdateIndex index ) override;
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
    inline const utf8String& listFlags();
    const tank::GameInstallationState& installationInfo();
    inline const Stats& stats() const throw() { return stats_; }
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