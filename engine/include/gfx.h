#pragma once
#include "forwards.h"
#include "gfx_types.h"
#include "neko_types.h"
#include "neko_platform.h"
#include "messaging.h"
#include "input.h"
#include "camera.h"

#include <MyGUI_NekoPlatform.h>

namespace neko {

  constexpr int32_t c_glVersion[2] = { 4, 6 };

  struct Viewport {
    vec2i size_;
    Viewport(): size_( 0, 0 ) {}
    Viewport( size_t width, size_t height ): size_( width, height ) {}
  };

  class Gfx:
    public platform::RenderWindowEventRecipient,
    public Listener
#ifndef NEKO_NO_GUI
    , public MyGUI::NekoImageLoader
#endif
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
  protected:
    Info info_;
    unique_ptr<sf::Window> window_;
    SceneNode* target_;
    unique_ptr<ArcballCamera> camera_;
    RendererPtr renderer_;
    Viewport viewport_;
    Image lastCapture_;
#ifndef NEKO_NO_GUI
    unique_ptr<MyGUI::NekoPlatform> guiPlatform_;
    unique_ptr<MyGUI::Gui> gui_;
#endif
    platform::RWLock logicLock_;
    void preInitialize();
    void printInfo();
    void resize( size_t width, size_t height );
    struct Flags {
      bool resized;
      bool reloadShaders;
      Flags(): resized( false ), reloadShaders( false ) {}
    } flags_;
  private:
    static void openglDebugCallbackFunction( GLenum source, GLenum type, GLuint id, GLenum severity,
      GLsizei length, const GLchar* message, const void* userParam );
    void setOpenGLDebugLogging( const bool enable );
    virtual void onMessage( const Message& msg );
#ifndef NEKO_NO_GUI
    // MyGUI::OpenGL3ImageLoader implementation
    void* loadImage( int& width, int& height, MyGUI::PixelFormat& format, const utf8String& filename ) override;
    void saveImage( int width, int height, MyGUI::PixelFormat format, void* texture, const utf8String& filename ) override;
#endif
  public:
    void postInitialize( Engine& engine );
    Gfx( ThreadedLoaderPtr loader, FontManagerPtr fonts, MessagingPtr messaging, DirectorPtr director, ConsolePtr console );
    const Image& renderWindowReadPixels() override;
    void processEvents(); //!< Process vital window events and such.
    void update( GameTime time, GameTime delta );
    inline Renderer& renderer() throw() { return *( renderer_.get() ); }
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
    GameTime lastTime_;
  protected:
    static bool threadProc( platform::Event& running, platform::Event& wantStop, void* argument );
    void initialize();
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