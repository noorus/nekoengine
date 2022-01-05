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

  using TextMap = map<size_t, js::Text*>;

  class TextManager: public enable_shared_from_this<TextManager>, public nocopy {
  private:
    ConsolePtr console_;
    TextMap texts_;
    void addJSText( js::Text* text );
    void removeJSText( js::Text* text );
  public:
    TextManager( ConsolePtr console ): console_( move( console ) ) {}
    void jsUpdate( RenderSyncContext& renderCtx );
    void jsReset();
    void teardown();
    inline TextMap& texts() { return texts_; }
  };

}