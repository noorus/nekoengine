#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "neko_types.h"
#include "neko_platform.h"

namespace neko {

  struct Viewport {
    vec2i size_;
    Viewport(): size_( 0, 0 ) {}
    Viewport( size_t width, size_t height ): size_( width, height ) {}
  };

  class Gfx: public Subsystem, public platform::RenderWindowEventRecipient {
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
    Gfx( EnginePtr engine );
    const Image& renderWindowReadPixels() override;
    void preUpdate( GameTime time ) override;
    void tick( GameTime tick, GameTime time ) override;
    void postUpdate( GameTime delta, GameTime tick ) override;
    void shutdown();
    void restart();
    virtual ~Gfx();
  };

}