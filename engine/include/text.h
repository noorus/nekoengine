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

  /*
  using TextMap = map<size_t, js::Text*>;

  class TextManager: public enable_shared_from_this<TextManager>, public nocopy {
  private:
    FontManagerPtr fonts_;
    ConsolePtr console_;
    TextMap texts_;
    void addJSText( js::Text* text );
    void removeJSText( js::Text* text );
  public:
    TextManager( FontManagerPtr fonts, ConsolePtr console ):
      console_( move( console ) ),
      fonts_( move( fonts ) ) {}
    void jsUpdate( RenderSyncContext& renderCtx );
    void jsReset();
    void teardown();
    inline TextMap& texts() { return texts_; }
  };*/

}