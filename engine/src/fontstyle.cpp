#include "pch.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "lodepng.h"
#include "renderer.h"
#include "engine.h"

namespace neko {

  FontStyle::FontStyle( FontFacePtr face, FT_Library ft, FT_Face ftface, Real size, vec2i atlasSize,
  FontRendering rendering, Real thickness, const unicodeString& prerenderGlyphs ): size_( size ),
  face_( face ), storedFaceIndex_( ftface->face_index ),
  rendering_( rendering ), outlineThickness_( thickness )
  {
    storedFaceSize_ = makeStoredFaceSize( size_ );

    auto fterr = FT_Set_Char_Size( face_->face_, 0, math::iround( size_ * c_fmagic ), c_dpi, c_dpi );
    if ( fterr )
      NEKO_FREETYPE_EXCEPT( "FreeType font character point size setting failed", fterr );

    //FT_Matrix matrix = { (int)( ( 1.0 ) * 0x10000L ), (int)( ( 0.0 ) * 0x10000L ), (int)( ( 0.0 ) * 0x10000L ),
    //  (int)( ( 1.0 ) * 0x10000L ) };

    FT_Set_Transform( face_->face_, nullptr, nullptr );

    hbfnt_ = hb_ft_font_create_referenced( face_->face_ );
    hb_ft_font_set_funcs( hbfnt_ ); // Doesn't create_referenced already call this?

    atlas_ = make_shared<TextureAtlas>( atlasSize, 1 );
    initEmptyGlyph();

    if ( !prerenderGlyphs.isEmpty() )
    {
      HBBuffer prerenderBuf( "en" );
      prerenderBuf.setFrom( hbfnt_, {}, prerenderGlyphs );
      for ( unsigned int i = 0; i < prerenderBuf.count(); ++i )
        getGlyph( ft, ftface, prerenderBuf.glyphInfo()[i].codepoint );
    }

    postLoad();
  }

  void FontStyle::initEmptyGlyph()
  {
    auto region = atlas_->getRegion( 5, 5 );
    if ( region.x < 0 )
      NEKO_EXCEPT( "Font face texture atlas is full" );

#pragma warning( push )
#pragma warning( disable: 4838 )
    static unsigned char data[4 * 4 * 3] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };
#pragma warning( pop )

    atlas_->setRegion( (int)region.x, (int)region.y, 4, 4, data, 0 );

    Glyph glyph;
    glyph.index = 0;
    glyph.coords[0] = vec2( region.x + 2, region.y + 2 ) / atlas_->fdimensions();
    glyph.coords[1] = vec2( region.x + 3, region.y + 3 ) / atlas_->fdimensions();

    glyphs_[0] = glyph;

    dirty_ = true;
  }

  void FontStyle::loadGlyph( FT_Library ft, FT_Face face, GlyphIndex index, bool hinting )
  {
    FT_Int32 flags = 0;
    flags |= FT_LOAD_DEFAULT;
    flags |= ( hinting ? FT_LOAD_FORCE_AUTOHINT : ( FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT ) );

    if ( atlas_->depth() == 3 )
    {
      FT_Library_SetLcdFilter( ft, FT_LCD_FILTER_DEFAULT );
      flags |= FT_LOAD_TARGET_LCD;
      uint8_t weights[5] = { 0x10, 0x40, 0x70, 0x40, 0x10 };
      FT_Library_SetLcdFilterWeights( ft, weights );
    }

    auto fterr = FT_Load_Glyph( face, index, flags );
    if ( fterr )
      NEKO_FREETYPE_EXCEPT( "FreeType glyph load error", fterr );

    FT_Bitmap bitmap;
    vec2i glyphCoords {};

    if ( rendering_ == FontRender_Normal )
    {
      FT_GlyphSlot slot = face->glyph;
      fterr = FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL );
      if ( fterr )
        NEKO_FREETYPE_EXCEPT( "FreeType glyph render error", fterr );
      bitmap = slot->bitmap;
      glyphCoords.x = slot->bitmap_left;
      glyphCoords.y = slot->bitmap_top;
    }
    else if ( rendering_ == FontRender_Outline_Expand )
    {
      FT_Stroker stroker;
      FT_Stroker_New( ft, &stroker );
      auto dist = static_cast<signed long>( outlineThickness_ * c_fmagic );
      FT_Stroker_Set( stroker, dist, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0 );

      FT_Glyph ftglyph;
      FT_Get_Glyph( face->glyph, &ftglyph );
      FT_Glyph_StrokeBorder( &ftglyph, stroker, false, true );
      FT_Glyph_To_Bitmap( &ftglyph, FT_RENDER_MODE_NORMAL, nullptr, true );
      auto bmglyph = reinterpret_cast<FT_BitmapGlyph>( ftglyph );
      bitmap = bmglyph->bitmap;
      glyphCoords.x = bmglyph->left;
      glyphCoords.y = bmglyph->top;
    }
    else
      NEKO_EXCEPT( "Unknown rendering mode" );

    vec4i padding( 0, 0, 0, 0 );

    auto src_w = static_cast<uint32_t>( bitmap.width / atlas_->depth() );
    auto src_h = static_cast<uint32_t>( bitmap.rows );
    auto tgt_w = src_w + static_cast<uint32_t>( padding.x + padding.z );
    auto tgt_h = src_h + static_cast<uint32_t>( padding.y + padding.w );

    auto region = atlas_->getRegion( tgt_w + 1, tgt_h + 1 );
    if ( region.x < 0 )
      NEKO_EXCEPT( "Font face texture atlas is full" );

    auto coord = vec2i( region.x, region.y );
    {
      Buffer tmp( static_cast<uint32_t>( tgt_w * tgt_h * atlas_->depth() ) );
      auto dst_ptr = tmp.data() + ( padding.y * tgt_w + padding.x ) * atlas_->depth();
      auto src_ptr = bitmap.buffer;
      for ( uint32_t i = 0; i < src_h; ++i )
      {
        memcpy( dst_ptr, src_ptr, bitmap.width );
        dst_ptr += tgt_w * atlas_->depth();
        src_ptr += bitmap.pitch;
      }

      atlas_->setRegion( (int)coord.x, (int)coord.y, (int)tgt_w, (int)tgt_h, tmp.data(),
        static_cast<size_t>( tgt_w ) * atlas_->depth() );
    }

    Glyph glyph;
    glyph.index = index;
    glyph.width = tgt_w;
    glyph.height = tgt_h;
    glyph.bearing = glyphCoords;
    glyph.coords[0] = vec2( coord ) / atlas_->fdimensions();
    glyph.coords[1] = vec2( coord.x + glyph.width, coord.y + glyph.height ) / atlas_->fdimensions();

    glyphs_[index] = glyph;

    dirty_ = true;
  }

  Glyph* FontStyle::getGlyph( FT_Library ft, FT_Face face, GlyphIndex index )
  {
    {
      const auto& glyph = glyphs_.find( index );
      if ( glyph != glyphs_.end() )
        return &( ( *glyph ).second );
    }
    loadGlyph( ft, face, index, true );
    {
      const auto& glyph = glyphs_.find( index );
      if ( glyph != glyphs_.end() )
        return &( ( *glyph ).second );
    }
    return nullptr;
  }

  void FontStyle::postLoad()
  {
    auto metrics = face_->face_->size->metrics;
    auto fmt = FT_Get_Font_Format( face_->face_ );
    if ( strcmp( fmt, "TrueType" ) == 0 )
    {
      ascender_ = static_cast<Real>( FT_MulFix( face_->face_->ascender, metrics.y_scale ) >> 6 );
      descender_ = static_cast<Real>( FT_MulFix( face_->face_->descender, metrics.y_scale ) >> 6 );
    }
    else
    {
      ascender_ = static_cast<Real>( metrics.ascender >> 6 );
      descender_ = static_cast<Real>( metrics.descender >> 6 );
    }
  }

  const TextureAtlas& FontStyle::atlas() const
  {
    assert( atlas_ );
    return *atlas_.get();
  }

  FontStyle::~FontStyle()
  {
    if ( hbfnt_ )
      hb_font_destroy( hbfnt_ );
    atlas_.reset();
  }

}