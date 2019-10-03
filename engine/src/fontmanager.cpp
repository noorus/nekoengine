#include "stdafx.h"
#include "locator.h"
#include "fontmanager.h"
#include "neko_exception.h"

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

  FontPtr FontManager::loadFace( uint8_t* buffer, size_t length, int32_t pointSize, uint32_t hDPI, uint32_t vDPI )
  {
    ScopedRWLock lock( &faceLock_ );

    FT_Open_Args args = { 0 };
    args.flags = FT_OPEN_MEMORY;
    args.memory_base = buffer;
    args.memory_size = (FT_Long)length;

    auto font = make_shared<Font>();
    auto error = FT_Open_Face( freeType_, &args, 0, &font->face_ );
    if ( error != 0 || !font->face_ )
    {
      lock.unlock();
      NEKO_EXCEPT( "FreeType font face load failed" );
    }

    forceUCS2Charmap( font->face_ );
    FT_Set_Char_Size( font->face_, 0, pointSize, hDPI, vDPI );

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

  Glyph FontManager::rasterize( FontPtr font, uint32_t glyphIndex )
  {
    if ( FT_Load_Glyph( font->face_, glyphIndex, FT_LOAD_DEFAULT ) != 0 )
      NEKO_EXCEPT( "FreeType glyph load failed" );

    auto slot = font->face_->glyph;
    if ( FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL ) != 0 )
      NEKO_EXCEPT( "FreeType glyph render failed" );

    auto bitmap = slot->bitmap;

    Glyph glyph;
    glyph.buffer = bitmap.buffer;
    glyph.width = bitmap.width;
    glyph.height = bitmap.rows;
    glyph.bearing_x = slot->bitmap_left;
    glyph.bearing_y = slot->bitmap_top;

    return move( glyph );
  }

  void FontManager::freeFace( FontPtr font )
  {
    ScopedRWLock lock( &faceLock_ );

    if ( font->face_ )
      FT_Done_Face( font->face_ );
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