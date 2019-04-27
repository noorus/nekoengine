#pragma once

namespace neko {

  class Engine;

  class Subsystem {
  public:
    Engine* engine_;
  public:
    Subsystem( Engine* engine ): engine_( engine ) {}
    virtual void gameBegin() = 0;
    virtual void gameEnd() = 0;
    virtual void draw() {}
  };

}