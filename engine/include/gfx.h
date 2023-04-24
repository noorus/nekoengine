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
#include "specialrenderers.h"

namespace neko {

  constexpr int32_t c_glVersion[2] = { 4, 6 };

  class ViewportDrawParameters {
  public:
    virtual bool drawopShouldDrawSky() const = 0;
    virtual bool drawopShouldDrawWireframe() const = 0;
    virtual vec2 drawopFullResolution() const = 0;
    virtual vec3 drawopClearColor() const = 0;
    virtual Real drawopExposure() const = 0;
  };

  class EditorViewport: public Viewport, public ViewportDrawParameters {
  protected:
    utf8String name_;
    unique_ptr<EditorOrthoCamera> camera_;
    bool panning_ = false;
    vec4 axisMask_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    vec2 windowResolution_ = { 0.0f, 0.0f };
    // EditorGridRenderer grid_;
  public:
    // ViewportDrawParameters interface implementation
    bool drawopShouldDrawSky() const override;
    bool drawopShouldDrawWireframe() const override;
    vec2 drawopFullResolution() const override;
    vec3 drawopClearColor() const override;
    Real drawopExposure() const override;
  public:
    EditorViewport( SceneManager* manager, vec2 resolution, const EditorViewportDefinition& def );
    void resize( int width, int height, const Viewport& windowViewport );
    inline void panning( bool isPanning ) { panning_ = isPanning; }
    inline bool panning() const { return panning_; }
    inline unique_ptr<EditorOrthoCamera>& camera() { return camera_; }
    inline const utf8String& name() const { return name_; }
    inline vec4 axisMask() const { return axisMask_; }
    vec3 pointToWorld( vec2 point ); // Cast a point on the 2D viewport to world coordinates. Depth will be zero.
    vec3 windowPointToWorld( vec2 point ); // Same as pointToWorld but assumes window coordinates
  };

  class GameViewport: public Viewport, public ViewportDrawParameters {
  protected:
    CameraPtr camera_;
    vec2 windowResolution_ = { 0.0f, 0.0f };
  public:
    void setCamera( CameraPtr camera );
    inline CameraPtr camera() const { return camera_; }
    void resize( int width, int height, const Viewport& windowViewport );
  public:
    // ViewportDrawParameters interface implementation
    bool drawopShouldDrawSky() const override;
    bool drawopShouldDrawWireframe() const override;
    vec2 drawopFullResolution() const override;
    vec3 drawopClearColor() const override;
    Real drawopExposure() const override;
  };

  class CursorLock {
  private:
    vec2i saved_ {};
  public:
    CursorLock()
    {
      platform::getCursorPosition( saved_ );
      platform::showCursor( false );
    }
    ~CursorLock()
    {
      platform::setCursorPosition( saved_ );
      platform::showCursor( true );
    }
  };

  class Editor {
  protected:
    bool enabled_ = true;
    vec3 clearColor_ = vec3( 0.0f, 0.0f, 0.0f );
    vector<EditorViewport> viewports_;
    int panningViewport_ = -1;
    unique_ptr<CursorLock> cursorLock_;
    float mainMenuHeight_ = 0.0f;
    vec2 mousePos_ { 0.0f };
  public:
    void initialize( RendererPtr renderer, const vec2& realResolution );
    void resize( const Viewport& windowViewport );
    inline void mainMenuHeight( float height ) { mainMenuHeight_ = height; }
    inline bool enabled() const { return enabled_; }
    inline void enabled( bool enable ) { enabled_ = enable; }
    inline vec3& clearColorRef() { return clearColor_; }
    void shutdown();
    void updateRealtime(
      GameTime realTime, GameTime delta, GfxInputPtr input, SceneManager& scene, const Viewport& window, OrbitCamera& gameCamera );
    bool draw( RendererPtr renderer, GameTime time, const Viewport& window, GameViewport& gameViewport );
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
    shared_ptr<OrbitCamera> camera_;
    RendererPtr renderer_;
    Viewport windowViewport_;
    GameViewport gameViewport_;
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
    void processEvents( bool discardMouse, bool discardKeyboard ); //!< Process vital window events and such.
    void updateRealTime( GameTime realTime, GameTime delta, Engine& engine );
    void preUpdate();
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