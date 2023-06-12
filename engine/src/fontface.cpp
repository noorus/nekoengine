#include "pch.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "lodepng.h"
#include "renderer.h"
#include "engine.h"

namespace neko {

  FontFace::FontFace( FontPtr font, FT_Library ft, FT_Open_Args* args, FaceID faceIndex, Real size ):
    ft_( ft ), font_( font ), size_( size )
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

    fterr = FT_Set_Char_Size( face_, 0, math::iround( size_ * c_fmagic ), c_dpi, c_dpi );
    if ( fterr )
      NEKO_FREETYPE_EXCEPT( "FreeType font character point size setting failed", fterr );

    FT_Matrix matrix = { (int)( ( 1.0 ) * 0x10000L ), (int)( ( 0.0 ) * 0x10000L ), (int)( ( 0.0 ) * 0x10000L ),
      (int)( ( 1.0 ) * 0x10000L ) };

    FT_Set_Transform( face_, &matrix, nullptr );

    hbfnt_ = hb_ft_font_create_referenced( face_ );
    hb_ft_font_set_funcs( hbfnt_ ); // Doesn't create_referenced already call this?

    postLoad();
  }

  StyleID FontFace::loadStyle( FontRendering rendering, Real thickness, const unicodeString& prerenderGlyphs )
  {
    auto id = makeStyleID( face_->face_index, size_, rendering, thickness );
    if ( styles_.find( id ) != styles_.end() )
      return id;

    auto atlasSize = vec2i( 1024 );

    auto style = make_shared<FontStyle>( ptr(), ft_, face_, makeStoredFaceSize( size_ ), atlasSize, rendering, thickness, prerenderGlyphs );

#ifdef _DEBUG
    auto cmp = style->id();
    FontStyleIndex sidx {};
    sidx.value = cmp;
    assert( sidx.value == id );
#endif

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

  void FontFace::postLoad()
  {
    auto metrics = face_->size->metrics;
    ascender_ = static_cast<Real>( metrics.ascender >> 6 );
    descender_ = static_cast<Real>( metrics.descender >> 6 );
    size_ = static_cast<Real>( metrics.height >> 6 );
  }

  Real FontFace::size() const
  {
    return size_;
  }

  Real FontFace::ascender() const
  {
    return ascender_;
  }

  Real FontFace::descender() const
  {
    return descender_;
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
    if ( hbfnt_ )
      hb_font_destroy( hbfnt_ );
  }

}