#pragma once
#include "forwards.h"
#include "gfx_types.h"
#include "neko_types.h"
#include "neko_platform.h"
#include "messaging.h"

namespace neko {

  struct Viewport {
    vec2i size_;
    Viewport(): size_( 0, 0 ) {}
    Viewport( size_t width, size_t height ): size_( width, height ) {}
  };

  struct FrameData {
    atomic<bool> frameCurrentlyProcessing = false;
  };

  struct Processor {
    static constexpr uint32_t maxQueuedFrames = 2;
    atomic<uint32_t> frameIndex;
    FrameData frameData[maxQueuedFrames];
  };

  class Gfx: public platform::RenderWindowEventRecipient {
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
  protected:
    Info info_;
    unique_ptr<sf::Window> window_;
    CameraPtr camera_;
    RendererPtr renderer_;
    Viewport viewport_;
    Image lastCapture_;
    void preInitialize();
    void printInfo();
    void resize( size_t width, size_t height );
    struct Flags {
      bool resized;
      Flags(): resized( false ) {}
    } flags_;
  private:
    static void openglDebugCallbackFunction( GLenum source, GLenum type, GLuint id, GLenum severity,
      GLsizei length, const GLchar* message, const void* userParam );
    void setOpenGLDebugLogging( const bool enable );
  public:
    void postInitialize();
    Gfx( ThreadedLoaderPtr loader, FontManagerPtr fonts, MessagingPtr messaging, ConsolePtr console );
    const Image& renderWindowReadPixels() override;
    void processEvents(); //!< Process vital window events and such.
    void preUpdate( GameTime time );
    void tick( GameTime tick, GameTime time );
    void postUpdate( GameTime delta, GameTime tick );
    void shutdown();
    void restart();
    virtual ~Gfx();
  };

  //struct FrameData {
  //};

  class FrameQueue {
  private:
    ReaderWriterQueue<FrameData> queue_;
  public:
    FrameQueue();
    ~FrameQueue();
  };

  class ThreadedRenderer: public enable_shared_from_this<ThreadedRenderer> {
  private:
    platform::Thread thread_;
    GfxPtr gfx_;
    ReaderWriterQueue<FrameData> frameQueue_;
  protected:
    ThreadedLoaderPtr loader_;
    FontManagerPtr fonts_;
    MessagingPtr messaging_;
    ConsolePtr console_;
  protected:
    static bool threadProc( platform::Event& running, platform::Event& wantStop, void* argument );
    void initialize();
    void run( platform::Event& wantStop );
  public:
    ThreadedRenderer( ThreadedLoaderPtr loader, FontManagerPtr fonts, MessagingPtr messaging, ConsolePtr console );
    void start();
    void stop();
    ~ThreadedRenderer();
  };

  using ThreadedRendererPtr = shared_ptr<ThreadedRenderer>;

}