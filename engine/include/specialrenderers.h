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

  class EditorGridRenderer {
  private:
    unique_ptr<LineRenderBuffer<1024>> viz_;
  public:
    EditorGridRenderer();
    ~EditorGridRenderer();
    void update( EditorOrthoCamera& camera );
    void draw( shaders::Shaders& shaders );
  };

}