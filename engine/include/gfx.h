#pragma once
#include "forwards.h"
#include "gfx_types.h"
#include "neko_types.h"
#include "neko_platform.h"
#include "messaging.h"
#include "input.h"
#include "camera.h"
#include "gui.h"
#include "viewport.h"

namespace neko {

  constexpr int32_t c_glVersion[2] = { 4, 6 };

  class EditorViewport: public Viewport {
  protected:
    utf8String name_;
    unique_ptr<EditorOrthoCamera> camera_;
  public:
    EditorViewport( SceneManager* manager, vec2 resolution, const EditorViewportDefinition& def );
    inline unique_ptr<EditorOrthoCamera>& camera() { return camera_; }
  };

  class Editor {
  protected:
    bool enabled_ = false;
    vec3 clearColor_ = vec3( 0.0f, 0.0f, 0.0f );
  public:
    inline bool enabled() const { return enabled_; }
    inline vec3& clearColorRef() { return clearColor_; }
  };

  class Gfx:
    public platform::RenderWindowEventRecipient,
    public Listener
  {
    friend class ThreadedRenderer;
  public:
    struct Info {
      string renderer_;
      string vendor_;
      string version_;
      Info() { clear(); }
      inline void clear()
      {
        renderer_ = "Unknown";
        version_ = "Unknown";
        vendor_ = "Unknown";
      }
    };
  protected:
    ConsolePtr console_;
    ThreadedLoaderPtr loader_;
    FontManagerPtr fonts_;
    MessagingPtr messaging_;
    DirectorPtr director_;
    GfxInputPtr input_;
    GUIPtr gui_;
  protected:
    Info info_;
    unique_ptr<sf::Window> window_;
    SceneNode* target_;
    unique_ptr<OrbitCamera> camera_;
    RendererPtr renderer_;
    Viewport windowViewport_;
    Viewport gameViewport_;
    vector<EditorViewport> viewports_;
    Image lastCapture_;
    Editor editor_;
    std::queue<uint64_t> updateAccounts_;
    platform::RWLock logicLock_;
    void preInitialize();
    void printInfo();
    void resize( size_t width, size_t height );
    struct Flags {
      bool editorResized = false;
      bool mainbufResized = false;
      bool reloadShaders = false;
    } flags_;
    void clear( const vec4& color );
  private:
    static void openglDebugCallbackFunction( GLenum source, GLenum type, GLuint id, GLenum severity,
      GLsizei length, const GLchar* message, const void* userParam );
    void setOpenGLDebugLogging( const bool enable );
    void onMessage( const Message& msg ) override;
  public:
    void postInitialize( Engine& engine );
    Gfx( ThreadedLoaderPtr loader, FontManagerPtr fonts, MessagingPtr messaging, DirectorPtr director, ConsolePtr console );
    const Image& renderWindowReadPixels() override;
    void processEvents(); //!< Process vital window events and such.
    void updateRealTime( GameTime realTime, GameTime delta, Engine& engine );
    void update( GameTime gameTime, GameTime delta, Engine& engine );
    inline Renderer& renderer() noexcept { return *( renderer_.get() ); }
    void shutdown();
    void restart( Engine& engine );
    void jsRestart();
    virtual ~Gfx();
  };

  class ThreadedRenderer: public enable_shared_from_this<ThreadedRenderer> {
  private:
    platform::Thread thread_;
    GfxPtr gfx_;
    platform::Event restartEvent_;
    platform::Event restartedEvent_;
  protected:
    EnginePtr engine_;
    ThreadedLoaderPtr loader_;
    FontManagerPtr fonts_;
    MessagingPtr messaging_;
    ConsolePtr console_;
    DirectorPtr director_;
    GameTime lastTime_ = 0.0;
    GameTime lastRealTime_ = 0.0;
  protected:
    static bool threadProc( platform::Event& running, platform::Event& wantStop, void* argument );
    void initialize();
    void shutdown();
    void run( platform::Event& wantStop );
  public:
    ThreadedRenderer( EnginePtr engine, ThreadedLoaderPtr loader, FontManagerPtr fonts, MessagingPtr messaging, DirectorPtr director, ConsolePtr console );
    void start();
    void restart();
    void stop();
    ~ThreadedRenderer();
  };

  using ThreadedRendererPtr = shared_ptr<ThreadedRenderer>;

}