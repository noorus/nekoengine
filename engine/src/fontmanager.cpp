#include "pch.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "engine.h"
#include "loader.h"

namespace neko {

  constexpr uint32_t c_newtypeApiVersion = 1;

#ifdef _DEBUG
  const wchar_t* c_newtypeLibraryName = L"newtype_d.dll";
#else
  const wchar_t* c_newtypeLibraryName = L"newtype.dll";
#endif

  void NewtypeLibrary::load( newtype::Host* host )
  {
    module_ = LoadLibraryW( c_newtypeLibraryName );
    if ( !module_ )
      NEKO_EXCEPT( "Newtype load failed" );
    pfnNewtypeInitialize = reinterpret_cast<newtype::fnNewtypeInitialize>( GetProcAddress( module_, "newtypeInitialize" ) );
    pfnNewtypeShutdown = reinterpret_cast<newtype::fnNewtypeShutdown>( GetProcAddress( module_, "newtypeShutdown" ) );
    if ( !pfnNewtypeInitialize || !pfnNewtypeShutdown )
      NEKO_EXCEPT( "Newtype export resolution failed" );
    manager_ = pfnNewtypeInitialize( c_newtypeApiVersion, host );
    if ( !manager_ )
      NEKO_EXCEPT( "Newtype init failed" );
  }

  void NewtypeLibrary::unload()
  {
    if ( manager_ && pfnNewtypeShutdown )
      pfnNewtypeShutdown( manager_ );
    if ( module_ )
      FreeLibrary( module_ );
  }

  void* FontManager::newtypeMemoryAllocate( uint32_t size )
  {
    return Locator::memory().alloc( Memory::Sector::Graphics, size );
  }

  void* FontManager::newtypeMemoryReallocate( void* address, uint32_t newSize )
  {
    return Locator::memory().realloc( Memory::Sector::Graphics, address, newSize );
  }

  void FontManager::newtypeMemoryFree( void* address )
  {
    Locator::memory().free( Memory::Sector::Graphics, address );
  }

  FontManager::FontManager( ThreadedLoaderPtr loader ):
  LoadedResourceManagerBase<Font>( loader )
  {
  }

  void FontManager::initializeLogic()
  {
    nt_.load( this );

    Locator::console().printf( Console::srcGfx, "Newtype reported: %s", nt_.mgr()->versionString().c_str() );
  }

  void FontManager::initializeRender( Renderer* renderer )
  {
    renderer_ = renderer;
  }

  FontPtr FontManager::createFont( const utf8String& name )
  {
    auto fnt = make_shared<Font>( nt_.mgr(), name );

    assert( fonts_.find( fnt->id() ) == fonts_.end() );
    fonts_[fnt->id()] = fnt;

    return fnt;
  }

  TextPtr FontManager::createText( const utf8String& fontName, newtype::StyleID style )
  {
    auto font = map_[fontName];

    auto txt = make_shared<Text>( nt_.mgr(), font, style );

    assert( texts_.find( txt->id() ) == texts_.end() );
    texts_[txt->id()] = txt;

    return txt;
  }

  void FontManager::newtypeFontTextureCreated( newtype::Font& ntfont, newtype::StyleID style, newtype::Texture& texture )
  {
    auto font = reinterpret_cast<Font*>( ntfont.getUser() );
  }

  void FontManager::newtypeFontTextureDestroyed( newtype::Font& ntfont, newtype::StyleID style, newtype::Texture& texture )
  {
    auto font = reinterpret_cast<Font*>( ntfont.getUser() );
  }

  void FontManager::prepareLogic( GameTime time )
  {
    //
  }

  void FontManager::loadJSONRaw( const nlohmann::json& obj )
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
      loader_->addLoadTask( { LoadTask( font, filename, 24.0f ) } );
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
    //FontVector fonts;
    //engine_->loader()->getFinishedFonts( fonts );

    for ( const auto& entry : fonts_ )
    {
      entry.second->update( renderer_ );
    }

    for ( const auto& entry : texts_ )
    {
      entry.second->update( renderer_ );
    }
  }

  void FontManager::draw()
  {
    assert( renderer_ );
    for ( auto& entry : texts_ )
    {
      entry.second->draw( renderer_ );
    }
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
    nt_.unload();
  }

  FontManager::~FontManager()
  {
    //
  }

}