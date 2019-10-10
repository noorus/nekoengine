#include "stdafx.h"
#include "locator.h"
#include "fontmanager.h"
#include "neko_exception.h"
#include "console.h"
#include "engine.h"
#include "loader.h"

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

  FontManager::FontManager( EnginePtr engine ): engine_( move( engine ) ), freeType_( nullptr )
  {
    ftMemAllocator_.user = nullptr;
    ftMemAllocator_.alloc = ftMemoryAllocate;
    ftMemAllocator_.realloc = ftMemoryReallocate;
    ftMemAllocator_.free = ftMemoryFree;
  }

  void FontManager::initialize()
  {
    assert( !freeType_ );

    if ( FT_New_Library( &ftMemAllocator_, &freeType_ ) != 0 )
      NEKO_EXCEPT( "FreeType library creation failed" );

    FT_Add_Default_Modules( freeType_ );
    // FT_Set_Default_Properties( freeType_ ); // TODO this uses an env variable? wtf, replace

    auto font = createFont();
    engine_->loader()->addLoadTask( { LoadTask( font, R"(data\fonts\DejaVuSerif.ttf)", 15.0f ) } );
  }

  void FontManager::uploadFonts()
  {
    FontVector fonts;
    engine_->loader()->getFinishedFonts( fonts );
    if ( !fonts.empty() )
      engine_->console()->printf( Console::srcGfx, "FontManager::uploadFonts got %d new fonts", fonts.size() );
    for ( auto& font : fonts )
    {
      if ( !font->loaded_ )
        continue;
      // guess we're done?
      font->impl_->loadGlyph( utils::utf8_to_utf32( "a" ) );
      font->impl_->loadGlyph( utils::utf8_to_utf32( "b" ) );
      font->impl_->loadGlyph( utils::utf8_to_utf32( "c" ) );
      font->impl_->loadGlyph( utils::utf8_to_utf32( "d" ) );
      font->impl_->loadGlyph( utils::utf8_to_utf32( "e" ) );
      font->impl_->loadGlyph( utils::utf8_to_utf32( "f" ) );
      font->impl_->loadGlyph( utils::utf8_to_utf32( "g" ) );
    }
  }

  void FontManager::prepare( GameTime time )
  {
    uploadFonts();
  }

  FontPtr FontManager::createFont()
  {
    ScopedRWLock lock( &faceLock_ );

    auto font = make_shared<Font>( this );
    fonts_.push_back( font );

    return move( font );
  }

  void FontManager::loadFont( FontPtr font, const Font::Specs& specs, vector<uint8_t>& buffer )
  {
    ScopedRWLock lock( &faceLock_ );

    const int atlasDepth = 1; // We'll just stick to this since we know what it means. (normal glyphs)

    assert( !font->loaded_ );

    font->impl_ = make_shared<fonts::GraphicalFont>( shared_from_this(),
      specs.atlasSize_.x, specs.atlasSize_.y, atlasDepth );
    font->impl_->loadFace( buffer, specs.pointSize_ );
    font->loaded_ = true;
  }

  void FontManager::shutdown()
  {
    ScopedRWLock lock( &faceLock_ );

    fonts_.clear();
    if ( freeType_ )
    {
      FT_Done_Library( freeType_ );
      freeType_ = nullptr;
    }
  }

  FontManager::~FontManager()
  {
    shutdown();
  }

}