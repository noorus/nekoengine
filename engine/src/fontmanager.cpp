#include "stdafx.h"
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

  FontManager::FontManager( EnginePtr engine ): engine_( engine )
  {
  }

  FontManager::~FontManager()
  {
    //
  }

  void FontManager::initializeRender( Renderer* renderer )
  {
    //
  }

  TextPtr FontManager::createText( FontPtr font, newtype::StyleID style )
  {
    auto txt = make_shared<Text>();
    txt->font_ = font;
    txt->text_ = nt_.mgr()->createText( font->face(), style );
    txt->text_->pen( vec3( 100.0f, 100.0f, 0.0f ) );
    auto content = unicodeString::fromUTF8( "It's pretty interesting.\nWhen you read ahead beforehand,\nyou have a much easier time in class." );
    txt->text_->setText( content );
    txts_.push_back( txt );
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

  void FontManager::prepareRender( Renderer* renderer )
  {
    for ( const auto& entry : ntfonts_ )
    {
      entry.second->update( renderer );
    }

    for ( const auto& txt : txts_ )
    {
      txt->update( renderer );
    }
  }

  void FontManager::draw( Renderer* renderer )
  {
    for ( auto& txt : txts_ )
    {
      const auto sid = txt->text_->styleid();
      if ( txt->mesh_
        && txt->font_
        && txt->font_->usable( sid ) )
      {
        txt->mesh_->draw( renderer->shaders(),
          txt->font_->style( sid ).material_->textureHandle( 0 )
        );
      }
    }
  }

}