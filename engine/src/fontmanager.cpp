#include "stdafx.h"
#include "locator.h"
#include "fontmanager.h"
#include "neko_exception.h"
#include "console.h"

namespace neko {

  static void* ftMemoryAllocate( FT_Memory memory, long size )
  {
    return Locator::memory().alloc( Memory::Graphics, size );
  }

  static void* ftMemoryReallocate( FT_Memory memory, long currentSize, long newSize, void* block )
  {
    return Locator::memory().realloc( Memory::Graphics, block, newSize );
  }

  void ftMemoryFree( FT_Memory memory, void* block )
  {
    Locator::memory().free( Memory::Graphics, block );
  }

  FontManager::FontManager(): freeType_( nullptr )
  {
    ftMemAllocator_.user = nullptr;
    ftMemAllocator_.alloc = ftMemoryAllocate;
    ftMemAllocator_.realloc = ftMemoryReallocate;
    ftMemAllocator_.free = ftMemoryFree;

    if ( FT_New_Library( &ftMemAllocator_, &freeType_ ) != 0 )
      NEKO_EXCEPT( "FreeType library creation failed" );

    FT_Add_Default_Modules( freeType_ );
    // FT_Set_Default_Properties( freeType_ ); // TODO this uses an env variable? wtf, replace
  }

  FontPtr g_testFont;

  void FontManager::initialize( size_t width, size_t height, size_t depth )
  {
    platform::FileReader reader( "DejaVuSerif.ttf" );
    vector<uint8_t> buffer;
    reader.readFullVector( buffer );

    atlas_ = make_shared<ftgl::TextureAtlas>( width, height, depth );
    g_testFont = move( loadFace( buffer.data(), buffer.size(), 15, atlas_ ) );

    g_testFont->loadGlyph( utils::utf8_to_utf32( "a" ) );
    g_testFont->loadGlyph( utils::utf8_to_utf32( "b" ) );
    g_testFont->loadGlyph( utils::utf8_to_utf32( "c" ) );
    g_testFont->loadGlyph( utils::utf8_to_utf32( "d" ) );
    g_testFont->loadGlyph( utils::utf8_to_utf32( "e" ) );
    g_testFont->loadGlyph( utils::utf8_to_utf32( "f" ) );
    g_testFont->loadGlyph( utils::utf8_to_utf32( "g" ) );
    fonts_.push_back( g_testFont );
  }

  void FontManager::shutdown()
  {
    if ( g_testFont )
    {
      freeFace( g_testFont );
    }
    g_testFont.reset();
    atlas_.reset();
  }

#define HRES  64
#define HRESf 64.f
#define DPI   72

  Font::Font( bool hinting, bool filtering, bool kerning ):
    hinting_( hinting ), filtering_( filtering ), kerning_( kerning ),
    ascender_( 0.0f ), descender_( 0.0f ), size_( 0.0f ),
    outline_thickness_( 0.0f ), padding_( 0 ),
    ftlib_( nullptr )
  {
    rendermode_ = ftgl::RENDER_NORMAL;

    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    lcd_weights[0] = 0x10;
    lcd_weights[1] = 0x40;
    lcd_weights[2] = 0x70;
    lcd_weights[3] = 0x40;
    lcd_weights[4] = 0x10;
  }

  void FontManager::textureFontPostInit( FontPtr font )
  {
    font->underline_position = math::round( font->face_->underline_position / (Real)( HRESf * HRESf ) * font->size_ );
    if ( font->underline_position > -2.0f )
      font->underline_position = -2.0f;

    font->underline_thickness = math::round( font->face_->underline_thickness / (Real)( HRESf * HRESf ) * font->size_ );
    if ( font->underline_thickness < 1.0f )
      font->underline_thickness = 1.0f;

    auto metrics = font->face_->size->metrics;
    font->ascender_ = ( metrics.ascender >> 6 ) / 100.0f;
    font->descender_ = ( metrics.descender >> 6 ) / 100.0f;
    font->size_ = ( metrics.height >> 6 ) / 100.0f;
    font->linegap_ = ( font->size_ - font->ascender_ + font->descender_ );
  }

  FontPtr FontManager::loadFace( uint8_t* buffer, size_t length, Real pointSize, ftgl::TextureAtlasPtr atlas )
  {
    ScopedRWLock lock( &faceLock_ );

    auto font = make_shared<Font>( true, true, true );
    font->data_.length_ = length;
    font->data_.buffer_ = static_cast<uint8_t*>( Locator::memory().alloc( Memory::Graphics, length ) );
    memcpy( font->data_.buffer_, buffer, length );

    font->atlas_ = atlas;
    font->ftlib_ = freeType_;
    font->size_ = pointSize;

    FT_Open_Args args = { 0 };
    args.flags = FT_OPEN_MEMORY;
    args.memory_base = font->data_.buffer_;
    args.memory_size = (FT_Long)font->data_.length_;

    auto err = FT_Open_Face( freeType_, &args, 0, &font->face_ );
    if ( err != 0 || !font->face_ )
    {
      lock.unlock();
      NEKO_FREETYPE_EXCEPT( "FreeType font face load failed", err );
    }

    Locator::console().printf( Console::srcEngine,
      "Font: %s, face %d/%d, glyphs: %d, charmaps: %d, scalable? %s",
      font->face_->family_name,
      0, font->face_->num_faces,
      font->face_->num_glyphs,
      font->face_->num_charmaps,
      ( font->face_->face_flags & FT_FACE_FLAG_SCALABLE ) ? "yes" : "no" );

    //forceUCS2Charmap( font->face_ );
    err = FT_Select_Charmap( font->face_, FT_ENCODING_UNICODE );
    if ( err )
      NEKO_FREETYPE_EXCEPT( "FreeType font charmap seelection failed", err );

    auto calcWidth = (signed long)( font->size_ * 100.0f * HRESf );
    const unsigned int horizRes = ( DPI * HRES );
    const unsigned int vertRes = ( DPI );
    err = FT_Set_Char_Size( font->face_, calcWidth, 0, horizRes, vertRes );
    if ( err )
      NEKO_FREETYPE_EXCEPT( "FreeType font character point size setting failed", err );

    FT_Matrix matrix = {
      (int)( ( 1.0 / HRES ) * 0x10000L ),
      (int)( ( 0.0 ) * 0x10000L ),
      (int)( ( 0.0 ) * 0x10000L ),
      (int)( ( 1.0 ) * 0x10000L ) };

    FT_Set_Transform( font->face_, &matrix, nullptr );

    textureFontPostInit( font );

    font->initEmptyGlyph();

    return move( font );
  }

  void FontManager::forceUCS2Charmap( FT_Face face )
  {
    // I have no idea what this is but someone said to do it.
    for ( auto i = 0; i < face->num_charmaps; ++i )
    {
      auto charmap = face->charmaps[i];
      if ( ( charmap->platform_id == 0 && charmap->encoding_id == 3 )
        || ( charmap->platform_id == 3 && charmap->encoding_id == 1 ) )
        if ( FT_Set_Charmap( face, charmap ) == 0 )
          return;
    }
  }

  void Font::initEmptyGlyph()
  {
    auto region = atlas_->getRegion( 5, 5 );
    if ( region.x < 0 )
      NEKO_EXCEPT( "Font face texture atlas is full" );

#pragma warning(push)
#pragma warning(disable:4838)
    static unsigned char data[4 * 4 * 3] = {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
#pragma warning(pop)

    atlas_->setRegion( region.x, region.y, 4, 4, data, 0 );

    ftgl::TextureGlyph glyph;
    glyph.codepoint = -1;
    glyph.coords[0].s = ( region.x + 2 ) / (Real)atlas_->width_;
    glyph.coords[0].t = ( region.y + 2 ) / (Real)atlas_->height_;
    glyph.coords[1].s = ( region.x + 3 ) / (Real)atlas_->width_;
    glyph.coords[1].t = ( region.y + 3 ) / (Real)atlas_->height_;

    glyphs_.push_back( move( glyph ) );
  }

  void Font::loadGlyph( uint32_t codepoint )
  {
    auto glyphIndex = FT_Get_Char_Index( face_, (FT_ULong)codepoint );

    FT_Int32 flags = 0;
    flags |= (
      ( rendermode_ != ftgl::RENDER_NORMAL && rendermode_ != ftgl::RENDER_SIGNED_DISTANCE_FIELD )
      ? FT_LOAD_NO_BITMAP : FT_LOAD_RENDER );
    flags |= ( hinting_ ? FT_LOAD_FORCE_AUTOHINT : ( FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT ) );

    if ( atlas_->depth_ == 3 )
    {
      FT_Library_SetLcdFilter( ftlib_, FT_LCD_FILTER_LIGHT );
      flags |= FT_LOAD_TARGET_LCD;
      if ( filtering_ )
        FT_Library_SetLcdFilterWeights( ftlib_, lcd_weights );
    }

    auto err = FT_Load_Glyph( face_, glyphIndex, flags );
    if ( err )
      NEKO_FREETYPE_EXCEPT( "FreeType glyph loading error", err );

    FT_Glyph ftglyph = nullptr;
    FT_GlyphSlot slot = nullptr;
    FT_Bitmap bitmap;
    vec2i glyphCoords;
    if ( rendermode_ == ftgl::RENDER_NORMAL || rendermode_ == ftgl::RENDER_SIGNED_DISTANCE_FIELD )
    {
      slot = face_->glyph;
      bitmap = slot->bitmap;
      glyphCoords.x = slot->bitmap_left;
      glyphCoords.y = slot->bitmap_top;
    }
    else
    {
      FT_Stroker stroker;
      err = FT_Stroker_New( ftlib_, &stroker );
      if ( err )
        NEKO_FREETYPE_EXCEPT( "FreeType stroker creation failed", err );

      FT_Stroker_Set( stroker, (int)( outline_thickness_ * HRES ), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0 );

      err = FT_Get_Glyph( face_->glyph, &ftglyph );
      if ( err )
        NEKO_FREETYPE_EXCEPT( "FreeType get glyph failed", err );

      if ( rendermode_ == ftgl::RENDER_OUTLINE_EDGE )
        err = FT_Glyph_Stroke( &ftglyph, stroker, 1 );
      else if ( rendermode_ == ftgl::RENDER_OUTLINE_POSITIVE )
        err = FT_Glyph_StrokeBorder( &ftglyph, stroker, 0, 1 );
      else if ( rendermode_ == ftgl::RENDER_OUTLINE_NEGATIVE )
        err = FT_Glyph_StrokeBorder( &ftglyph, stroker, 1, 1 );

      if ( err )
        NEKO_FREETYPE_EXCEPT( "FreeType glyph stroke failed", err );

      if ( atlas_->depth_ == 1 )
        err = FT_Glyph_To_Bitmap( &ftglyph, FT_RENDER_MODE_NORMAL, 0, 1 );
      else
        err = FT_Glyph_To_Bitmap( &ftglyph, FT_RENDER_MODE_LCD, 0, 1 );

      if ( err )
        NEKO_FREETYPE_EXCEPT( "FreeType glyph to bitmap failed", err );

      auto bitmapGlyph = (FT_BitmapGlyph)ftglyph;
      bitmap = bitmapGlyph->bitmap;
      glyphCoords.x = bitmapGlyph->left;
      glyphCoords.y = bitmapGlyph->top;

      FT_Stroker_Done( stroker );
    }

    vec4i padding( 0, 0, 1, 1 );
    if ( rendermode_ == ftgl::RENDER_SIGNED_DISTANCE_FIELD )
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

    auto region = atlas_->getRegion( tgt_w, tgt_h );
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

    if ( rendermode_ == ftgl::RENDER_SIGNED_DISTANCE_FIELD )
    {
      // TODO make_distance_mapb
    }

    atlas_->setRegion( x, y, tgt_w, tgt_h, buffer, tgt_w * atlas_->depth_ );
    Locator::memory().free( Memory::Graphics, buffer );

    ftgl::TextureGlyph glyph;
    glyph.codepoint = codepoint;
    glyph.width = tgt_w;
    glyph.height = tgt_h;
    glyph.rendermode = rendermode_;
    glyph.offset = glyphCoords;
    glyph.coords[0].s = x / (float)atlas_->width_;
    glyph.coords[0].t = y / (float)atlas_->height_;
    glyph.coords[1].s = ( x + glyph.width ) / (float)atlas_->width_;
    glyph.coords[1].t = ( y + glyph.height ) / (float)atlas_->height_;

    FT_Load_Glyph( face_, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_HINTING );
    slot = face_->glyph;
    glyph.advance.x = (Real)slot->advance.x / HRESf;
    glyph.advance.y = (Real)slot->advance.y / HRESf;

    glyphs_.push_back( move( glyph ) );

    if ( ftglyph )
      FT_Done_Glyph( ftglyph );

    generateKerning();
  }

  void Font::generateKerning()
  {
    assert( glyphs_.size() > 0 );

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

  void FontManager::freeFace( FontPtr font )
  {
    ScopedRWLock lock( &faceLock_ );

    if ( font->face_ )
      FT_Done_Face( font->face_ );
    if ( font->data_.buffer_ )
      Locator::memory().free( Memory::Graphics, font->data_.buffer_ );
  }

  FontManager::~FontManager()
  {
    if ( freeType_ )
    {
      FT_Done_Library( freeType_ );
      freeType_ = nullptr;
    }
  }

}