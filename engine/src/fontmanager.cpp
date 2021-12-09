#include "stdafx.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "engine.h"
#include "loader.h"

namespace neko {

  void* ftMemoryAllocate( FT_Memory memory, long size );
  void* ftMemoryReallocate( FT_Memory memory, long currentSize, long newSize, void* block );
  void ftMemoryFree( FT_Memory memory, void* block );

  FontManager::FontManager( EnginePtr engine ):
  engine_( move( engine ) ), freeType_( nullptr ), ftVersion_({ 0 }), hbVersion_({ 0 })
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

    FT_Library_Version( freeType_, &ftVersion_.major, &ftVersion_.minor, &ftVersion_.patch );
    hb_version( &hbVersion_.major, &hbVersion_.minor, &hbVersion_.patch );
    engine_->console()->printf( Console::srcGfx,
      "Using FreeType v%i.%i.%i, HarfBuzz v%i.%i.%i",
      ftVersion_.major, ftVersion_.minor, ftVersion_.patch,
      hbVersion_.major, hbVersion_.minor, hbVersion_.patch );

    ftVersion_.trueTypeSupport = FT_Get_TrueType_Engine_Type( freeType_ );
    engine_->console()->printf( Console::srcGfx, "TrueType engine: %s",
      ftVersion_.trueTypeSupport == FT_TRUETYPE_ENGINE_TYPE_PATENTED ? "full"
      : ftVersion_.trueTypeSupport == FT_TRUETYPE_ENGINE_TYPE_UNPATENTED ? "partial"
      : "none" );

    // FT_Set_Default_Properties( freeType_ ); // TODO this uses an env variable? wtf, replace
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
      // could preload glyphs here, if we knew which ones.
    }
  }

  void FontManager::prepare( GameTime time )
  {
    uploadFonts();
  }

  FontPtr FontManager::createFont()
  {
    ScopedRWLock lock( &faceLock_ );

    auto font = make_shared<Font>( shared_from_this() );
    fonts_.push_back( font );

    return move( font );
  }

  void FontManager::loadFont( FontPtr font, const Font::Specs& specs, vector<uint8_t>& buffer )
  {
    ScopedRWLock lock( &faceLock_ );

    const int atlasDepth = 1; // We'll just stick to this since we know what it means. (normal glyphs)

    assert( !font->loaded_ );

    font->loadFace( buffer, specs.pointSize_, vec3i( specs.atlasSize_, atlasDepth ) );
    font->loaded_ = true;
  }

  void FontManager::unloadFont( Font* font )
  {
    /* if ( font && font->impl_ )
    {
      font->impl_.reset();
      font->loaded_ = false;
    }*/
  }

  void FontManager::shutdown()
  {
    ScopedRWLock lock( &faceLock_ );

    for ( auto& font : fonts_ )
    {
      unloadFont( font.get() );
    }

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

  // FreeType allocator interface implementation

  void* ftMemoryAllocate( FT_Memory memory, long size )
  {
    return Locator::memory().alloc( Memory::Sector::Graphics, size );
  }

  void* ftMemoryReallocate( FT_Memory memory, long currentSize, long newSize, void* block )
  {
    return Locator::memory().realloc( Memory::Sector::Graphics, block, newSize );
  }

  void ftMemoryFree( FT_Memory memory, void* block )
  {
    Locator::memory().free( Memory::Sector::Graphics, block );
  }

}