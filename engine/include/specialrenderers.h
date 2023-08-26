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
#include "buffers.h"

namespace neko {

  class EditorViewport;

  class EditorGridRenderer {
  private:
    unique_ptr<LineRenderBuffer<1024>> viz_;
    int drawCount_ = 0;
    mat4 mdl;
  public:
    EditorGridRenderer();
    ~EditorGridRenderer();
    void update( const EditorViewport& viewport, EditorOrthoCamera& camera );
    void draw( Shaders& shaders );
  };

  class AxesPointerRenderer {
  private:
    unique_ptr<LineRenderBuffer<6>> viz_;
  public:
    AxesPointerRenderer();
    ~AxesPointerRenderer();
    void draw( Shaders& shaders, vec3 origin, vec3 up, vec3 right );
  };

}