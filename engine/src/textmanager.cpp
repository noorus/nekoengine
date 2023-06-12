#include "pch.h"

#include "gfx_types.h"
#include "modelmanager.h"
#include "renderer.h"
#include "neko_exception.h"
#include "console.h"
#include "font.h"
#include "text.h"

namespace neko {

  /*
  using namespace gl;

  void TextManager::addJSText( js::Text* text )
  {
    if ( texts_.find( text->text().id_ ) != texts_.end() )
    {
      NEKO_EXCEPT( "Model map key already exists" );
    }
    texts_[text->text().id_] = text;
  }

  void TextManager::removeJSText( js::Text* text )
  {
    texts_.erase( text->text().id_ );
    delete text;
  }

  void TextManager::jsUpdate( RenderSyncContext& renderCtx )
  {
    js::TextVector inTexts;
    js::TextVector outTexts;
    renderCtx.syncTextsFromRenderer( inTexts, outTexts );
    if ( !inTexts.empty() || !outTexts.empty() )
    {
      Locator::console().printf( Console::srcGfx,
        "TextManager::jsUpdate adding %i texts, removing %i texts",
        inTexts.size(), outTexts.size() );
    }
    for ( auto& txt : inTexts )
    {
      addJSText( txt );
    }
    for ( auto& txt : outTexts )
    {
      removeJSText( txt );
    }
  }

  void TextManager::jsReset()
  {
    for ( auto& it : texts_ )
    {
      delete it.second;
    }
    texts_.clear();
  }

  void TextManager::teardown()
  {
    jsReset();
  }*/

}