#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"

namespace neko {

  NEKO_EXTERN_CONVAR( vid_screenwidth );
  NEKO_EXTERN_CONVAR( vid_screenheight );
  NEKO_EXTERN_CONVAR( gl_debuglog );

  class Gfx: public Subsystem {
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
    void preInitialize();
    void printInfo();
  private:
    static void openglDebugCallbackFunction( GLenum source, GLenum type, GLuint id, GLenum severity,
      GLsizei length, const GLchar* message, const void* userParam );
    void setOpenGLDebugLogging( const bool enable );
  public:
    void postInitialize();
    Gfx( EnginePtr engine );
    virtual void preUpdate( GameTime time ) override;
    virtual void tick( GameTime tick, GameTime time ) override;
    virtual void postUpdate( GameTime delta, GameTime tick ) override;
    void shutdown();
    void restart();
    virtual ~Gfx();
  };

}