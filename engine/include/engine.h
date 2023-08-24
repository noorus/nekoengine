#pragma once
#include "neko_types.h"
#include "neko_filepath.h"
#include "forwards.h"
#include "subsystem.h"
#include "console.h"
#include "messaging.h"
#include "director.h"

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
  class Engine: public ShareableBase<Engine>, public nocopy, public Listener
  {
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
    SteamPtr steam_;
    EngineInfo info_;
    Environment env_;
  protected:
    struct State {
      bool focusLost = false;
      bool windowMove = false;
      bool steamOverlay = false;
      bool timePaused = false;
      bool devMode = true;
    } state_;
    struct Sync
    {
      atomic<GameTime> gameTime = 0.0;
      atomic<GameTime> realTime = 0.0;
    } sync_;
    Stats stats_;
    platform::PerformanceClock clock_;
    GameTime time_ = 0.0;
    GameTime realTime_ = 0.0;
    volatile Signal signal_ = Signal_None;
    EngineSettings settings_;
    bool paused();
    void restart();
  protected:
    //! Message listener callback.
    void onMessage( const Message& msg ) override;
  public:
    inline const EngineInfo& info() const noexcept { return info_; }
    inline ConsolePtr console() noexcept { return console_; }
    inline Environment& env() noexcept { return env_; }
#ifndef NEKO_NO_SCRIPTING
    inline ScriptingPtr scripting() noexcept { return scripting_; }
#endif
    inline GameTime time() const noexcept { return time_; }
    inline GameTime realTime() const noexcept { return realTime_; }
    inline ThreadedLoaderPtr loader() noexcept { return loader_; }
    inline FontManagerPtr fonts() noexcept { return fonts_; }
    inline MessagingPtr msgs() noexcept { return messaging_; }
    inline DirectorPtr director() noexcept { return director_; }
    inline Sync& sync() noexcept { return sync_; }
    inline const utf8String& listFlags();
    inline const Stats& stats() const noexcept { return stats_; }
    inline const bool devmode() const noexcept { return state_.devMode; }
  public:
    //! Constructor.
    Engine( ConsolePtr console, const Environment& env );
    //! Destructor.
    ~Engine();
    void requestQuit();
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