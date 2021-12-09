#include "stdafx.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "renderer.h"

namespace neko {

    constexpr int c_dpi = 72;

    Font::Font( FontManagerPtr manager )
        :
      manager_( move( manager ) ), face_( nullptr ),
      hinting_( true ), filtering_( true ), kerning_( true ),
      ascender_( 0.0f ), descender_( 0.0f ), size_( 0.0f ),
      outline_thickness_( 0.0f ), padding_( 0 ),
      rendermode_( FontRenderMode::Normal ),
      lcd_weights{ 0x10, 0x40, 0x70, 0x40, 0x10 }
    {
      // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
      // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    }

    void Font::loadFace( vector<uint8_t>& source, Real pointSize, vec3i atlasSize )
    {
      assert( !data_.get() );
      assert( size_ < 1.0f );

      atlas_ = make_shared<TextureAtlas>( atlasSize.x, atlasSize.y, atlasSize.z );

      data_ = make_unique<utils::DumbBuffer>( Memory::Sector::Graphics, source );
      size_ = pointSize;

      auto ftlib = manager_->library();

      FT_Open_Args args = { 0 };
      args.flags = FT_OPEN_MEMORY;
      args.memory_base = data_->data();
      args.memory_size = (FT_Long)data_->length();

      auto err = FT_Open_Face( ftlib, &args, 0, &face_ );
      if ( err || !face_ )
      {
        NEKO_FREETYPE_EXCEPT( "FreeType font face load failed", err );
      }

      Locator::console().printf( Console::srcGfx,
        "Font: %s, face %d/%d, glyphs: %d, charmaps: %d, scalable? %s",
        face_->family_name,
        0, face_->num_faces,
        face_->num_glyphs,
        face_->num_charmaps,
        ( face_->face_flags & FT_FACE_FLAG_SCALABLE ) ? "yes" : "no" );

      //forceUCS2Charmap( font->face_ );
      err = FT_Select_Charmap( face_, FT_ENCODING_UNICODE );
      if ( err )
        NEKO_FREETYPE_EXCEPT( "FreeType font charmap selection failed", err );


      err = FT_Set_Char_Size( face_, 0, math::iround( size_ * 64.0f ), resolution_ * c_dpi, c_dpi );
      if ( err )
        NEKO_FREETYPE_EXCEPT( "FreeType font character point size setting failed", err );

      FT_Matrix matrix = {
        (int)( ( 1.0 / static_cast<double>( resolution_ ) ) * 0x10000L ),
        (int)( ( 0.0 ) * 0x10000L ),
        (int)( ( 0.0 ) * 0x10000L ),
        (int)( ( 1.0 ) * 0x10000L ) };

      FT_Set_Transform( face_, &matrix, nullptr );

      hbfnt_ = hb_ft_font_create_referenced( face_ );

      postLoad();
      initEmptyGlyph();
    }

    Font::~Font()
    {
      if ( hbfnt_ )
        hb_font_destroy( hbfnt_ );
      if ( face_ )
        FT_Done_Face( face_ );
    }

    void Font::forceUCS2Charmap()
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

    void Font::postLoad()
    {
      underline_position = math::round( face_->underline_position / (Real)( resolution_ * resolution_ ) * size_ );
      if ( underline_position > -2.0f )
        underline_position = -2.0f;

      underline_thickness = math::round( face_->underline_thickness / (Real)( resolution_ * resolution_ ) * size_ );
      if ( underline_thickness < 1.0f )
        underline_thickness = 1.0f;

      auto metrics = face_->size->metrics;
      ascender_ = static_cast<Real>( metrics.ascender >> 6 );
      descender_ = static_cast<Real>( metrics.descender >> 6 );
      size_ = static_cast<Real>( metrics.height >> 6 );
      linegap_ = ( size_ - ascender_ + descender_ );
    }

    void Font::initEmptyGlyph()
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

      TexturedGlyph glyph;
      glyph.codepoint = -1;
      glyph.coords[0].x = ( region.x + 2 ) / (Real)atlas_->width_;
      glyph.coords[0].y = ( region.y + 2 ) / (Real)atlas_->height_;
      glyph.coords[1].x = ( region.x + 3 ) / (Real)atlas_->width_;
      glyph.coords[1].y = ( region.y + 3 ) / (Real)atlas_->height_;

      glyphs_[0] = move( glyph );
    }

    void Font::loadGlyph( Codepoint codepoint )
    {
      auto ftlib = manager_->library();
      auto glyphIndex = FT_Get_Char_Index( face_, (FT_ULong)codepoint );

      FT_Int32 flags = 0;
      flags |= ( ( rendermode_ != FontRenderMode::Normal && rendermode_ != FontRenderMode::SDF ) ? FT_LOAD_NO_BITMAP : FT_LOAD_RENDER );
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
      if ( rendermode_ == FontRenderMode::Normal || rendermode_ == FontRenderMode::SDF )
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

        FT_Stroker_Set( stroker, (int)( outline_thickness_ * resolution_ ), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0 );

        err = FT_Get_Glyph( face_->glyph, &ftglyph );
        if ( err )
          NEKO_FREETYPE_EXCEPT( "FreeType get glyph failed", err );

        if ( rendermode_ == FontRenderMode::OutlineEdge )
          err = FT_Glyph_Stroke( &ftglyph, stroker, 1 );
        else if ( rendermode_ == FontRenderMode::OutlineOuter )
          err = FT_Glyph_StrokeBorder( &ftglyph, stroker, 0, 1 );
        else if ( rendermode_ == FontRenderMode::OutlineInner )
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
      if ( rendermode_ == FontRenderMode::SDF )
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

      auto buffer = (uint8_t*)Locator::memory().alloc( Memory::Sector::Graphics, tgt_w * tgt_h * atlas_->depth_ );
      auto dst_ptr = buffer + ( padding.y * tgt_w + padding.x ) * atlas_->depth_;
      auto src_ptr = bitmap.buffer;
      for ( int i = 0; i < src_h; ++i )
      {
        memcpy( dst_ptr, src_ptr, bitmap.width );
        dst_ptr += tgt_w * atlas_->depth_;
        src_ptr += bitmap.pitch;
      }

      if ( rendermode_ == FontRenderMode::SDF )
      {
        // TODO make_distance_mapb
      }

      atlas_->setRegion( x, y, tgt_w, tgt_h, buffer, tgt_w * atlas_->depth_ );
      Locator::memory().free( Memory::Sector::Graphics, buffer );

      TexturedGlyph glyph;
      glyph.codepoint = codepoint;
      glyph.width = tgt_w;
      glyph.height = tgt_h;
      glyph.rendermode = rendermode_;
      glyph.offset = glyphCoords;
      glyph.coords[0].x = x / (Real)atlas_->width_;
      glyph.coords[0].y = y / (Real)atlas_->height_;
      glyph.coords[1].x = ( x + glyph.width ) / (Real)atlas_->width_;
      glyph.coords[1].y = ( y + glyph.height ) / (Real)atlas_->height_;

      FT_Load_Glyph( face_, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_HINTING );
      slot = face_->glyph;
      glyph.advance.x = (Real)slot->advance.x / static_cast<Real>( resolution_ );
      glyph.advance.y = (Real)slot->advance.y / static_cast<Real>( resolution_ );

      glyphs_[codepoint] = move( glyph );

      if ( ftglyph )
        FT_Done_Glyph( ftglyph );

      generateKerning();
    }

    TexturedGlyph* Font::getGlyph( Codepoint codepoint )
    {
      {
        const auto& glyph = glyphs_.find( codepoint );
        if ( glyph != glyphs_.end() )
          return &( ( *glyph ).second );
      }
      loadGlyph( codepoint );
      {
        const auto& glyph = glyphs_.find( codepoint );
        if ( glyph != glyphs_.end() )
          return &( ( *glyph ).second );
      }
      return nullptr;
    }

    void Font::generateKerning()
    {
      for ( auto& glyphp : glyphs_ )
      {
        if ( glyphp.first == 0 )
          continue;

        auto& glyph = glyphp.second;
        auto index = FT_Get_Char_Index( face_, glyph.codepoint );
        glyph.kerning.clear();

        for ( auto& other : glyphs_ )
        {
          if ( other.first == 0 )
            continue;
          const auto& prev_glyph = other.second;
          const auto prev_index = FT_Get_Char_Index( face_, prev_glyph.codepoint );
          FT_Vector kerning;
          FT_Get_Kerning( face_, prev_index, index, FT_KERNING_UNFITTED, &kerning );
          if ( kerning.x )
            glyph.kerning[prev_glyph.codepoint] = ( kerning.x / (Real)( static_cast<Real>( resolution_ ) * static_cast<Real>( resolution_ ) ) );
        }
      }
    }

    bool Font::use( Renderer* renderer, GLuint textureUnit )
    {
      if ( !material_ )
      {
        char matname[128];
        sprintf_s( matname, 128, "font/%s/%i", face_->family_name, static_cast<int>( size_ * 100.0f ) );
        material_ = renderer->createTextureWithData(
          matname, atlas_->width_, atlas_->height_,
          PixFmtColorR8, atlas_->data_.data(),
          Texture::ClampBorder, Texture::Nearest );
        /*platform::FileWriter writer( "debug.png" );
        vector<uint8_t> buffer;
        lodepng::encode( buffer, font_->impl_->atlas_->data_, font_->impl_->atlas_->width_, font_->impl_->atlas_->height_, LCT_GREY, 8 );
        writer.writeBlob( buffer.data(), buffer.size() );*/
      }
      if ( material_ )
      {
        gl::glBindTextureUnit( 0, material_->layers_[0].texture_->handle() );
        return true;
      }
      return false;
    }

}