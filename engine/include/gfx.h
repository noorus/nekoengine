#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"

namespace neko {

  struct GfxContextInfo {
    short glVersionMinor; //!< Major GL version
    short glVersionMajor; //!< Minor GL version
    short depthBits; //!< Depth buffer bits per pixel
    short stencilBits; //!< Stencil buffer bits per pixel
    inline bool hasDepth() const throw( ) { return ( depthBits > 0 ); }
    inline bool hasStencil() const throw( ) { return ( stencilBits > 0 ); }
  };

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