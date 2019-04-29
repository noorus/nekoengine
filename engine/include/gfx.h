#pragma once
#include "subsystem.h"
#include "forwards.h"

namespace neko {

  class Gfx: public Subsystem {
  protected:
    SDL_Window* window_;
    SDL_Surface* screenSurface_;
    SDL_Renderer* renderer_;
    SDL_DisplayMode displayMode_;
    SDL_GLContext glContext_;
    ShadersPtr shaders_;
    void preInitialize();
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