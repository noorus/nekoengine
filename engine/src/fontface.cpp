#include "pch.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "lodepng.h"
#include "renderer.h"
#include "engine.h"

namespace neko {

  FontFace::FontFace( FontPtr font, FT_Library ft, FT_Open_Args* args, FaceID faceIndex ):
    ft_( ft ), font_( font )
  {
    auto fterr = FT_Open_Face( ft, args, faceIndex, &face_ );
    if ( fterr || !face_ )
      NEKO_FREETYPE_EXCEPT( "FreeType font face load failed", fterr );

    forceUCS2Charmap();

    fterr = FT_Select_Charmap( face_, FT_ENCODING_UNICODE );
    if ( fterr )
      NEKO_FREETYPE_EXCEPT( "FreeType font charmap selection failed", fterr );

    if ( !FT_IS_SCALABLE( face_ ) )
      NEKO_EXCEPT( "Font is not scalable; bitmap fonts unsupported" );
  }

  StyleID FontFace::loadStyle( FontRendering rendering, Real sz, Real thickness, const unicodeString& prerenderGlyphs )
  {
    auto id = makeStyleID( face_->face_index, sz, rendering, thickness );
    if ( styles_.find( id ) != styles_.end() )
      return id;

    auto atlasSize = vec2i( 1024 );

    auto style = make_shared<FontStyle>( ptr(), ft_, face_, sz, atlasSize, rendering, thickness, prerenderGlyphs );

#ifdef _DEBUG
    auto cmp = style->id();
    FontStyleIndex sidx {};
    sidx.value = cmp;
    assert( sidx.value == id );
#endif

    Locator::console().printf( srcGfx, "FONT setting style id %I64X to 0x%I64X", style->id(), *style );
    styles_[style->id()] = style;
    return style->id();
  }

  void FontFace::forceUCS2Charmap()
  {
    assert( face_ );

    for ( auto i = 0; i < face_->num_charmaps; ++i )
    {
      auto charmap = face_->charmaps[i];
      if ( ( charmap->platform_id == 0 && charmap->encoding_id == 3 ) ||
           ( charmap->platform_id == 3 && charmap->encoding_id == 1 ) )
        if ( FT_Set_Charmap( face_, charmap ) == 0 )
          return;
    }
  }

  FontStylePtr FontFace::style( StyleID id )
  {
    auto it = styles_.find( id );
    if ( it == styles_.end() )
      return {};
    return it->second;
  }

  FontFace::~FontFace()
  {
    styles_.clear();
  }

}