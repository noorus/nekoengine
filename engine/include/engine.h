#pragma once
#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "console.h"
#include "messaging.h"
#include "director.h"
#include "rainet.h"

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

  struct RainetLibrary {
  public:
    HMODULE module_;
    rainet::System* engine_;
    rainet::fnInitialize pfnInitialize;
    rainet::fnShutdown pfnShutdown;
    void load( rainet::Host* host );
    void unload();
  };

  struct Stats {
    atomic<int> i_launches = 0;
    atomic<float> f_timeWasted = 0.0f;
  };

  struct EngineSettings {
    bool useSteam = false;
    uint32_t steamAppID = 0;
    bool useDiscord = false;
    uint64_t discordAppID = 0;
  };

  //! \class Engine
  //! The main engine class that makes the world go round
  class Engine: public enable_shared_from_this<Engine>, public nocopy, public Listener, public rainet::Host {
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
    ThreadedRendererPtr renderer_;
#ifndef NEKO_NO_SCRIPTING
    ScriptingPtr scripting_;
#endif
    ThreadedLoaderPtr loader_;
    FontManagerPtr fonts_;
    MessagingPtr messaging_;
    DirectorPtr director_;
    EngineInfo info_;
    RainetLibrary rainet_;
    Environment env_;
  protected:
    struct State {
      bool focusLost;
      bool windowMove;
      bool steamOverlay;
      bool timePaused;
      State(): focusLost( false ), windowMove( false ), steamOverlay( false ), timePaused( false ) {}
    } state_;
    Stats stats_;
    platform::PerformanceClock clock_;
    GameTime time_;
    volatile Signal signal_;
    atomic<GameTime> rendererTime_;
    EngineSettings settings_;
    bool paused();
    void restart();
  protected:
    //! Message listener callback.
    void onMessage( const Message& msg ) override;
    //! Rainet callbacks.
    void onDiscordDebugPrint( const utf8String& message ) override;
    void onSteamDebugPrint( const utf8String& message ) override;
    void onSteamOverlayToggle( bool enabled ) override;
    void onSteamStatsUpdated( rainet::StateUpdateIndex index ) override;
    void onAccountUpdated( const rainet::Account& user ) override;
  public:
    inline const EngineInfo& info() const noexcept { return info_; }
    inline ConsolePtr console() noexcept { return console_; }
    inline Environment& env() noexcept { return env_; }
#ifndef NEKO_NO_SCRIPTING
    inline ScriptingPtr scripting() noexcept { return scripting_; }
#endif
    inline GameTime time() const noexcept { return time_; }
    inline ThreadedLoaderPtr loader() noexcept { return loader_; }
    inline FontManagerPtr fonts() noexcept { return fonts_; }
    inline MessagingPtr msgs() noexcept { return messaging_; }
    inline DirectorPtr director() noexcept { return director_; }
    inline atomic<GameTime>& renderTime() noexcept { return rendererTime_; }
    inline const utf8String& listFlags();
    const rainet::GameInstallationState& installationInfo();
    inline const Stats& stats() const noexcept { return stats_; }
    inline rainet::System* rainet() { return rainet_.engine_; }
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