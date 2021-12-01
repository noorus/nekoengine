#pragma once
#include "forwards.h"
#include "gfx_types.h"
#include "neko_types.h"
#include "neko_platform.h"
#include "messaging.h"
#include "input.h"
#include "camera.h"
#include "gui.h"
#include "font.h"
#include "loader.h"

namespace neko {

  class Text {
  public:
    DynamicMeshPtr mesh_;
    FontPtr font_;
    utf8String text_;
    vec2 pen_;
    bool dirty_;
  public:
    Text( ThreadedLoaderPtr loader, MeshManagerPtr meshmgr, FontPtr font );
    void set( const utf8String& text, vec2 pen );
    void regenerate();
    void draw( Renderer* renderer );
    ~Text();
  };

}