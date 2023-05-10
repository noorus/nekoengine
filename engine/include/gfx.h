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
    virtual vec4 drawopGridColor() const = 0;
    virtual Real drawopExposure() const = 0;
    virtual const CameraPtr drawopGetCamera() const = 0;
    virtual void drawopPreSceneDraw( shaders::Shaders& shaders ) const = 0;
    virtual void drawopPostSceneDraw( shaders::Shaders& shaders ) const = 0;
  };

  class Editor;
  using EditorPtr = shared_ptr<Editor>;

  class EditorViewport;

  using EditorViewportPtr = shared_ptr<EditorViewport>;

  struct ViewportDragOperation
  {
  protected:
    EditorViewportPtr vp_;
    bool dragging_ = false;
    vec2 startPos_;
    vec2 worlddims_;
    vec2 curPos_;
    vec2 remapNormal( vec2 vpcoord ) const; //!< Maps -1...1 coord to 0...1
    vec2 remapWorld( vec2 norm ) const; //!< Maps 0...1 to world dimensions
  public:
    void begin( EditorViewportPtr vp, vec2 wincoord );
    void update( vec2 wincoord );
    inline const bool ongoing() const noexcept { return dragging_; }
    inline EditorViewportPtr viewport() const { return vp_; }
    vec2 getRelative() const;
    void end();
  };

  class EditorViewport: public Viewport, public ViewportDrawParameters {
  protected:
    utf8String name_;
    shared_ptr<EditorOrthoCamera> camera_;
    bool panning_ = false;
    vec4 axisMask_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    vec2 windowResolution_ = { 0.0f, 0.0f };
    unique_ptr<EditorGridRenderer> grid_;
    unique_ptr<AxesPointerRenderer> axesGizmo_;
    EditorPtr editor_; 
  public:
    // ViewportDrawParameters interface implementation
    bool drawopShouldDrawSky() const override;
    bool drawopShouldDrawWireframe() const override;
    vec2 drawopFullResolution() const override;
    vec3 drawopClearColor() const override;
    vec4 drawopGridColor() const override;
    Real drawopExposure() const override;
    void drawopPreSceneDraw( shaders::Shaders& shaders ) const override;
    void drawopPostSceneDraw( shaders::Shaders& shaders ) const override;
    const CameraPtr drawopGetCamera() const override;
    // Viewport interface implementation
    vec3 ndcPointToWorld( vec2 ndc_viewcoord ) const override;
    vec3 ndcPointToWorld( vec3 ndc_viewcoord ) const override;
  public:
    EditorViewport( SceneManager* manager, EditorPtr editor, vec2 resolution, const EditorViewportDefinition& def );
    ~EditorViewport();
    void resize( int width, int height, const Viewport& windowViewport );
    inline void panning( bool isPanning ) { panning_ = isPanning; }
    inline bool panning() const { return panning_; }
    inline shared_ptr<EditorOrthoCamera>& camera() { return camera_; }
    inline const utf8String& name() const { return name_; }
    inline vec4 axisMask() const { return axisMask_; }
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
    vec4 drawopGridColor() const override;
    Real drawopExposure() const override;
    void drawopPreSceneDraw( shaders::Shaders& shaders ) const override;
    void drawopPostSceneDraw( shaders::Shaders& shaders ) const override;
    const CameraPtr drawopGetCamera() const override;
    // Viewport interface implementation
    vec3 ndcPointToWorld( vec2 ndc_viewcoord ) const override;
    vec3 ndcPointToWorld( vec3 ndc_viewcoord ) const override;
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

  class Editor: public enable_shared_from_this<Editor> {
  protected:
    bool enabled_ = true;
    vec3 clearColor_ = vec3( 0.0f, 0.0f, 0.0f );
    vec4 gridColor_ = vec4( 0.06f, 0.055f, 0.062f, 0.38f );
    vector<EditorViewportPtr> viewports_;
    int panningViewport_ = -1;
    unique_ptr<CursorLock> cursorLock_;
    float mainMenuHeight_ = 0.0f;
    vec2 mousePos_ { 0.0f };
    ViewportDragOperation dragOp_;
    vec2 lastDragopPos_ { 0.0f, 0.0f };
  public:
    void initialize( RendererPtr renderer, const vec2& realResolution );
    void resize( const Viewport& windowViewport, GameViewport& gameViewport );
    inline void mainMenuHeight( float height ) { mainMenuHeight_ = height; }
    inline bool enabled() const { return enabled_; }
    inline void enabled( bool enable ) { enabled_ = enable; }
    inline vec3& clearColorRef() { return clearColor_; }
    inline vec4& gridColorRef() { return gridColor_; }
    void shutdown();
    void updateRealtime( GameTime realTime, GameTime delta, GfxInputPtr input, SceneManager& scene, const Viewport& window,
      GameViewport& gameViewport );
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
    EditorPtr editor_;
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