#include "stdafx.h"
#include "locator.h"
#include "fontmanager.h"
#include "neko_exception.h"
#include "console.h"

namespace neko {

  namespace fonts {

#define HRES  64
#define HRESf 64.f
#define DPI   72

    const Real c_resolutionMultiplier = 4.0f;

    GraphicalFont::GraphicalFont( FontManagerPtr manager, size_t width, size_t height, size_t depth ):
      manager_( move( manager ) ), face_( nullptr ),
      hinting_( true ), filtering_( true ), kerning_( true ),
      ascender_( 0.0f ), descender_( 0.0f ), size_( 0.0f ),
      outline_thickness_( 0.0f ), padding_( 0 ),
      rendermode_( RENDER_NORMAL )
    {
      // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
      // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
      lcd_weights[0] = 0x10;
      lcd_weights[1] = 0x40;
      lcd_weights[2] = 0x70;
      lcd_weights[3] = 0x40;
      lcd_weights[4] = 0x10;

      atlas_ = make_shared<TextureAtlas>( width, height, depth );
    }

    void GraphicalFont::loadFace( vector<uint8_t>& source, Real pointSize )
    {
      assert( !data_.get() );
      assert( size_ < 1.0f );

      data_ = make_unique<utils::DumbBuffer>( Memory::Graphics, source );
      size_ = pointSize;

      auto ftlib = manager_->lib();

      FT_Open_Args args = { 0 };
      args.flags = FT_OPEN_MEMORY;
      args.memory_base = data_->data();
      args.memory_size = (FT_Long)data_->length();

      auto err = FT_Open_Face( ftlib, &args, 0, &face_ );
      if ( err || !face_ )
      {
        NEKO_FREETYPE_EXCEPT( "FreeType font face load failed", err );
      }

      Locator::console().printf( Console::srcEngine,
        "Font: %s, face %d/%d, glyphs: %d, charmaps: %d, scalable? %s",
        face_->family_name,
        0, face_->num_faces,
        face_->num_glyphs,
        face_->num_charmaps,
        ( face_->face_flags & FT_FACE_FLAG_SCALABLE ) ? "yes" : "no" );

      //forceUCS2Charmap( font->face_ );
      err = FT_Select_Charmap( face_, FT_ENCODING_UNICODE );
      if ( err )
        NEKO_FREETYPE_EXCEPT( "FreeType font charmap seelection failed", err );

      auto calcWidth = (signed long)( size_ * c_resolutionMultiplier * HRESf );
      const unsigned int horizRes = ( DPI * HRES );
      const unsigned int vertRes = ( DPI );
      err = FT_Set_Char_Size( face_, calcWidth, 0, horizRes, vertRes );
      if ( err )
        NEKO_FREETYPE_EXCEPT( "FreeType font character point size setting failed", err );

      // I have absolutely no idea what this is but seems important.
      FT_Matrix matrix = {
        (int)( ( 1.0 / HRES ) * 0x10000L ),
        (int)( ( 0.0 ) * 0x10000L ),
        (int)( ( 0.0 ) * 0x10000L ),
        (int)( ( 1.0 ) * 0x10000L ) };

      FT_Set_Transform( face_, &matrix, nullptr );

      postInit();
      initEmptyGlyph();
    }

    GraphicalFont::~GraphicalFont()
    {
      if ( face_ )
        FT_Done_Face( face_ );
    }

    void GraphicalFont::forceUCS2Charmap()
    {
      assert( face_ );

      for ( auto i = 0; i < face_->num_charmaps; ++i )
      {
        auto charmap = face_->charmaps[i];
        if ( ( charmap->platform_id == 0 && charmap->encoding_id == 3 )
          || ( charmap->platform_id == 3 && charmap->encoding_id == 1 ) )
          if ( FT_Set_Charmap( face_, charmap ) == 0 )
            return;
      }
    }

    void GraphicalFont::postInit()
    {
      underline_position = math::round( face_->underline_position / (Real)( HRESf * HRESf ) * size_ );
      if ( underline_position > -2.0f )
        underline_position = -2.0f;

      underline_thickness = math::round( face_->underline_thickness / (Real)( HRESf * HRESf ) * size_ );
      if ( underline_thickness < 1.0f )
        underline_thickness = 1.0f;

      auto metrics = face_->size->metrics;
      ascender_ = ( metrics.ascender >> 6 ) / c_resolutionMultiplier;
      descender_ = ( metrics.descender >> 6 ) / c_resolutionMultiplier;
      size_ = ( metrics.height >> 6 ) / c_resolutionMultiplier;
      linegap_ = ( size_ - ascender_ + descender_ );
    }

    void GraphicalFont::initEmptyGlyph()
    {
      auto region = atlas_->getRegion( 5, 5 );
      if ( region.x < 0 )
        NEKO_EXCEPT( "Font face texture atlas is full" );

#pragma warning( push )
#pragma warning( disable: 4838 )
      static unsigned char data[4 * 4 * 3] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
      };
#pragma warning( pop )

      atlas_->setRegion( region.x, region.y, 4, 4, data, 0 );

      TextureGlyph glyph;
      glyph.codepoint = -1;
      glyph.coords[0].s = ( region.x + 2 ) / (Real)atlas_->width_;
      glyph.coords[0].t = ( region.y + 2 ) / (Real)atlas_->height_;
      glyph.coords[1].s = ( region.x + 3 ) / (Real)atlas_->width_;
      glyph.coords[1].t = ( region.y + 3 ) / (Real)atlas_->height_;

      glyphs_.push_back( move( glyph ) );
    }

    void GraphicalFont::loadGlyph( uint32_t codepoint )
    {
      auto ftlib = manager_->lib();
      auto glyphIndex = FT_Get_Char_Index( face_, (FT_ULong)codepoint );

      FT_Int32 flags = 0;
      flags |= (
        ( rendermode_ != RENDER_NORMAL && rendermode_ != RENDER_SIGNED_DISTANCE_FIELD )
        ? FT_LOAD_NO_BITMAP : FT_LOAD_RENDER );
      flags |= ( hinting_ ? FT_LOAD_FORCE_AUTOHINT : ( FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT ) );

      if ( atlas_->depth_ == 3 )
      {
        FT_Library_SetLcdFilter( ftlib, FT_LCD_FILTER_LIGHT );
        flags |= FT_LOAD_TARGET_LCD;
        if ( filtering_ )
          FT_Library_SetLcdFilterWeights( ftlib, lcd_weights );
      }

      auto err = FT_Load_Glyph( face_, glyphIndex, flags );
      if ( err )
        NEKO_FREETYPE_EXCEPT( "FreeType glyph loading error", err );

      FT_Glyph ftglyph = nullptr;
      FT_GlyphSlot slot = nullptr;
      FT_Bitmap bitmap;
      vec2i glyphCoords;
      if ( rendermode_ == RENDER_NORMAL || rendermode_ == RENDER_SIGNED_DISTANCE_FIELD )
      {
        slot = face_->glyph;
        bitmap = slot->bitmap;
        glyphCoords.x = slot->bitmap_left;
        glyphCoords.y = slot->bitmap_top;
      }
      else
      {
        FT_Stroker stroker;
        err = FT_Stroker_New( ftlib, &stroker );
        if ( err )
          NEKO_FREETYPE_EXCEPT( "FreeType stroker creation failed", err );

        FT_Stroker_Set( stroker, (int)( outline_thickness_ * HRES ), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0 );

        err = FT_Get_Glyph( face_->glyph, &ftglyph );
        if ( err )
          NEKO_FREETYPE_EXCEPT( "FreeType get glyph failed", err );

        if ( rendermode_ == RENDER_OUTLINE_EDGE )
          err = FT_Glyph_Stroke( &ftglyph, stroker, 1 );
        else if ( rendermode_ == RENDER_OUTLINE_POSITIVE )
          err = FT_Glyph_StrokeBorder( &ftglyph, stroker, 0, 1 );
        else if ( rendermode_ == RENDER_OUTLINE_NEGATIVE )
          err = FT_Glyph_StrokeBorder( &ftglyph, stroker, 1, 1 );

        if ( err )
          NEKO_FREETYPE_EXCEPT( "FreeType glyph stroke failed", err );

        if ( atlas_->depth_ == 1 )
          err = FT_Glyph_To_Bitmap( &ftglyph, FT_RENDER_MODE_NORMAL, nullptr, 1 );
        else
          err = FT_Glyph_To_Bitmap( &ftglyph, FT_RENDER_MODE_LCD, nullptr, 1 );

        if ( err )
          NEKO_FREETYPE_EXCEPT( "FreeType glyph to bitmap failed", err );

        auto bitmapGlyph = (FT_BitmapGlyph)ftglyph;
        bitmap = bitmapGlyph->bitmap;
        glyphCoords.x = bitmapGlyph->left;
        glyphCoords.y = bitmapGlyph->top;

        FT_Stroker_Done( stroker );
      }

      vec4i padding( 0, 0, 0, 0 );
      if ( rendermode_ == RENDER_SIGNED_DISTANCE_FIELD )
      {
        padding.x = 1;
        padding.y = 1;
      }

      if ( padding_ != 0 )
        padding += padding_;

      size_t src_w = bitmap.width / atlas_->depth_;
      size_t src_h = bitmap.rows;
      size_t tgt_w = src_w + padding.x + padding.z;
      size_t tgt_h = src_h + padding.y + padding.w;

      auto region = atlas_->getRegion( tgt_w + 4, tgt_h + 4 );
      if ( region.x < 0 )
        NEKO_EXCEPT( "Font face texture atlas is full" );

      auto x = region.x;
      auto y = region.y;

      auto buffer = (uint8_t*)Locator::memory().alloc( Memory::Graphics, tgt_w * tgt_h * atlas_->depth_ );
      auto dst_ptr = buffer + ( padding.y * tgt_w + padding.x ) * atlas_->depth_;
      auto src_ptr = bitmap.buffer;
      for ( int i = 0; i < src_h; ++i )
      {
        memcpy( dst_ptr, src_ptr, bitmap.width );
        dst_ptr += tgt_w * atlas_->depth_;
        src_ptr += bitmap.pitch;
      }

      if ( rendermode_ == RENDER_SIGNED_DISTANCE_FIELD )
      {
        // TODO make_distance_mapb
      }

      atlas_->setRegion( x, y, tgt_w, tgt_h, buffer, tgt_w * atlas_->depth_ );
      Locator::memory().free( Memory::Graphics, buffer );

      TextureGlyph glyph;
      glyph.codepoint = codepoint;
      glyph.width = tgt_w;
      glyph.height = tgt_h;
      glyph.rendermode = rendermode_;
      glyph.offset = glyphCoords;
      glyph.coords[0].s = x / (Real)atlas_->width_;
      glyph.coords[0].t = y / (Real)atlas_->height_;
      glyph.coords[1].s = ( x + glyph.width ) / (Real)atlas_->width_;
      glyph.coords[1].t = ( y + glyph.height ) / (Real)atlas_->height_;

      FT_Load_Glyph( face_, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_HINTING );
      slot = face_->glyph;
      glyph.advance.x = (Real)slot->advance.x / HRESf;
      glyph.advance.y = (Real)slot->advance.y / HRESf;

      glyphs_.push_back( move( glyph ) );

      if ( ftglyph )
        FT_Done_Glyph( ftglyph );

      generateKerning();
    }

    void GraphicalFont::generateKerning()
    {
      assert( !glyphs_.empty() );

      for ( size_t i = 1; i < glyphs_.size(); ++i )
      {
        auto glyph = &glyphs_[i];
        auto index = FT_Get_Char_Index( face_, glyph->codepoint );
        glyph->kerning.clear();

        for ( size_t j = 1; j < glyphs_.size(); ++j )
        {
          auto prev_glyph = &glyphs_[j];
          auto prev_index = FT_Get_Char_Index( face_, prev_glyph->codepoint );

          FT_Vector kerning;
          FT_Get_Kerning( face_, prev_index, index, FT_KERNING_UNFITTED, &kerning );
          if ( kerning.x )
          {
            Kerning k = { prev_glyph->codepoint, kerning.x / (Real)( HRESf * HRESf ) };
            glyph->kerning.push_back( move( k ) );
          }
        }
      }
    }

  }

}