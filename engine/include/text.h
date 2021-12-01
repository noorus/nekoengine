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

  class DynamicText {
  public:
    DynamicMeshPtr mesh_;
    FontPtr font_;
    MaterialPtr material_;
    utf8String text_;
    vec2 pen_;
    bool meshGenerated_;
  public:
    DynamicText( ThreadedLoaderPtr loader, MeshManagerPtr meshmgr, FontManagerPtr fontmgr );
    void setText( const utf8String& text, vec2 pen );
    void regenerate();
    void draw( Renderer* renderer );
    ~DynamicText();
  };

}