#include "pch.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "engine.h"
#include "loader.h"

namespace neko {

  void* ftMemoryAllocate( FT_Memory memory, long size )
  {
    return Locator::memory().alloc( Memory::Sector::Fonts, size );
  }

  void* ftMemoryReallocate( FT_Memory memory, long currentSize, long newSize, void* block )
  {
    return Locator::memory().realloc( Memory::Sector::Fonts, block, newSize );
  }

  void ftMemoryFree( FT_Memory memory, void* block )
  {
    Locator::memory().free( Memory::Sector::Fonts, block );
  }

  FontManager::FontManager( ThreadedLoaderPtr loader ):
  LoadedResourceManagerBase<Font>( loader )
  {
    ftMemAllocator_.user = this;
    ftMemAllocator_.alloc = ftMemoryAllocate;
    ftMemAllocator_.realloc = ftMemoryReallocate;
    ftMemAllocator_.free = ftMemoryFree;
  }

  void FontManager::initializeLogic()
  {
    assert( !freeType_ );

    auto fterr = FT_New_Library( &ftMemAllocator_, &freeType_ );
    if ( fterr )
      NEKO_FREETYPE_EXCEPT( "FreeType library creation failed", fterr );

    FT_Add_Default_Modules( freeType_ );

    FT_Library_Version( freeType_, &ftVersion_.major, &ftVersion_.minor, &ftVersion_.patch );
    hb_version( &hbVersion_.major, &hbVersion_.minor, &hbVersion_.patch );

    hb_icu_get_unicode_funcs();

    ftVersion_.trueTypeSupport = FT_Get_TrueType_Engine_Type( freeType_ );

    Locator::console().printf( Console::srcGfx, "Using FreeType v%i.%i.%i HarfBuzz v%i.%i.%i",
      ftVersion_.major, ftVersion_.minor,
      ftVersion_.patch, hbVersion_.major, hbVersion_.minor, hbVersion_.patch );
  }

  void FontManager::initializeRender( Renderer* renderer )
  {
    renderer_ = renderer;
  }

  FontPtr FontManager::createFont( const utf8String& name )
  {
    auto font = make_shared<Font>( ptr(), fontIndex_++, name );
    fonts_[name] = font;

    return move( font );
  }

  FontFacePtr FontManager::loadFace( FontPtr font, span<uint8_t> buffer, FaceID faceIndex )
  {
    return font->loadFace( buffer, faceIndex );
  }

  StyleID FontManager::loadStyle(
    FontFacePtr face, Real size, FontRendering rendering, Real thickness, const unicodeString& prerenderGlyphs )
  {
    return face->loadStyle( rendering, size, thickness, prerenderGlyphs );
  }

  void FontManager::unloadFont( FontPtr font )
  {
    font->unload();
    for ( auto it = fonts_.begin(); it != fonts_.end(); )
      if ( ( *it ).second->id() == font->id() )
        it = fonts_.erase( it );
      else
        ++it;
  }

  TextPtr FontManager::createText( FontStylePtr style )
  {
    auto nid = textIndex_++;
    Text::Features feats { .ligatures = true, .kerning = true };
    auto text = make_shared<Text>( ptr(), nid, style, feats );
    texts_[nid] = text;
    return move( text );
  }

  void FontManager::prepareLogic( GameTime time )
  {
    //
  }

  void FontManager::loadJSONRaw( const json& obj )
  {
    if ( obj.is_array() )
    {
      for ( const auto& entry : obj )
      {
        if ( !entry.is_object() )
          NEKO_EXCEPT( "Font array entry is not an object" );
        loadJSONRaw( entry );
      }
    }
    else if ( obj.is_object() )
    {
      auto name = obj["name"].get<utf8String>();
      if ( map_.find( name ) != map_.end() )
        NEKO_EXCEPT( "Font already exists: " + name );
      auto font = createFont( name );
      auto filename = obj["file"].get<utf8String>();
      map_[font->name()] = font;
      auto sizes = obj["preloadSizes"];
      if ( !sizes.is_array() )
        NEKO_EXCEPT( "preloadSizes is not an array" );
      vector<FontLoadSpec> specs;
      for ( const auto& size : sizes )
      {
        if ( !size.is_number() )
          NEKO_EXCEPT( "preloadSizes array entry is not a number" );
        specs.emplace_back( size.get<Real>(), FontRender_Normal, 0.0f );
      }
      loader_->addLoadTask( { LoadTask( font, filename, specs ) } );
    }
    else
      NEKO_EXCEPT( "Font JSON is not an array or an object" );
  }

  void FontManager::loadJSON( const utf8String& input )
  {
    auto parsed = nlohmann::json::parse( input );
    loadJSONRaw( parsed );
  }

  void FontManager::loadFile( const utf8String& filename )
  {
    auto input = move( Locator::fileSystem().openFile( Dir_Data, filename )->readFullString() );
    loadJSON( input );
  }

  void FontManager::prepareRender()
  {
    assert( renderer_ );

    FontVector fonts;
    loader_->getFinishedFonts( fonts );
    if ( !fonts.empty() )
      Locator::console().printf( Console::srcGfx, "FontManager::prepareRender got %d new fonts", fonts.size() );

    for ( auto& newFont : fonts )
    {
      if ( fonts_.find( newFont->name() ) == fonts_.end() )
        fonts_[newFont->name()] = newFont;
      else if ( fonts_[newFont->name()] != newFont )
        NEKO_EXCEPT( "Loader-returned font already exists" );
    }

    for ( const auto& [key, font] : fonts_ )
    {
      font->update( renderer_ );
    }

    /* for ( const auto& entry : texts_ )
    {
      entry.second->update( renderer_ );
    }*/
  }

  void FontManager::update()
  {
    for ( auto it = texts_.begin(); it != texts_.end(); )
      if ( ( *it ).second->dead() )
        it = texts_.erase( it );
      else
      {
        it->second->update( *renderer_ );
        ++it;
      }
  }

  void FontManager::draw()
  {
    /* assert( renderer_ );
    for ( auto& entry : texts_ )
    {
      entry.second->draw( renderer_ );
    }*/
  }

  void FontManager::shutdownRender()
  {
    texts_.clear();
    map_.clear();
    fonts_.clear();
    renderer_ = nullptr;
  }

  void FontManager::shutdownLogic()
  {
    fonts_.clear();
    if ( freeType_ )
    {
      FT_Done_Library( freeType_ );
      freeType_ = nullptr;
    }
  }

  FontManager::~FontManager()
  {
  }

}