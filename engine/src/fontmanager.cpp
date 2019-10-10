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

  fonts::FontPtr g_testFont;

  void FontManager::initialize()
  {
    platform::FileReader reader( "DejaVuSerif.ttf" );
    vector<uint8_t> buffer;
    reader.readFullVector( buffer );

    g_testFont = make_shared<fonts::Font>( shared_from_this(), 8192, 8192, 1 );
    g_testFont->loadFace( buffer, 15.0f );

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
    g_testFont.reset();
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