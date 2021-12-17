#include "stdafx.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "lodepng.h"
#include "renderer.h"

namespace neko {

  void NewtypeLibrary::load( newtype::Host* host )
  {
    module_ = LoadLibraryW( L"newtype_d.dll" );
    if ( !module_ )
      NEKO_EXCEPT( "Newtype load failed" );
    pfnNewtypeInitialize = reinterpret_cast<newtype::fnNewtypeInitialize>( GetProcAddress( module_, "newtypeInitialize" ) );
    pfnNewtypeShutdown = reinterpret_cast<newtype::fnNewtypeShutdown>( GetProcAddress( module_, "newtypeShutdown" ) );
    if ( !pfnNewtypeInitialize || !pfnNewtypeShutdown )
      NEKO_EXCEPT( "Newtype export resolution failed" );
    manager_ = pfnNewtypeInitialize( 1, host );
    if ( !manager_ )
      NEKO_EXCEPT( "TankEngine init failed" );
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

  void FontManager::initialize()
  {
    nt_.load( this );
    fnt_ = nt_.mgr()->createFont();
    vector<uint8_t> input;
    Locator::fileSystem().openFile( Dir_Fonts, R"(SourceHanSansJP-Bold.otf)" )->readFullVector( input );
    nt_.mgr()->loadFont( fnt_, input, 24.0f );
    txt_ = nt_.mgr()->createText( fnt_ );
    txt_->pen( vec3( 100.0f, 100.0f, 0.0f ) );
    auto content = unicodeString::fromUTF8( "It's pretty interesting.\nWhen you read ahead beforehand,\nyou have a much easier time in class." );
    txt_->setText( content );
  }

  void FontManager::newtypeFontTextureCreated( newtype::Font& font, newtype::Texture& texture )
  {
    FontData data;
    fontdata_[font.id()] = move( data );
  }

  void FontManager::newtypeFontTextureDestroyed( newtype::Font& font, newtype::Texture& texture )
  {
    fontdata_.erase( font.id() );
  }

  void FontManager::prepareLogic( GameTime time )
  {
    //
  }

  void FontManager::prepareRender( Renderer* renderer )
  {
    for ( auto& font : nt_.mgr()->fonts() )
    {
      if ( !font->loaded() || !font->dirty() )
        continue;
      auto& data = fontdata_[font->id()];
      data.material_ = renderer->createTextureWithData( "fonttest",
        font->texture().dimensions().x, font->texture().dimensions().y,
        PixFmtColorR8, font->texture().data(),
        Texture::ClampBorder, Texture::Nearest );
      platform::FileWriter writer( "fonttest_atlas.png" );
      vector<uint8_t> buffer;
      lodepng::encode( buffer,
        font->texture().data(),
        static_cast<unsigned int>( font->texture().dimensions().x ),
        static_cast<unsigned int>( font->texture().dimensions().y ),
        LCT_GREY, 8 );
      writer.writeBlob( buffer.data(), static_cast<uint32_t>( buffer.size() ) );
      font->markClean();
    }
    if ( txt_ )
    {
      txt_->update();
      if ( txt_->dirty() && mesh_ )
        mesh_.reset();
      if ( !txt_->dirty() && !mesh_ )
      {
        mesh_ = make_unique<TextRenderBuffer>(
          static_cast<gl::GLuint>( txt_->mesh().vertices_.size() ),
          static_cast<gl::GLuint>( txt_->mesh().indices_.size() ) );
        const auto& verts = mesh_->buffer().lock();
        const auto& indcs = mesh_->indices().lock();
        memcpy( verts.data(), txt_->mesh().vertices_.data(), txt_->mesh().vertices_.size() * sizeof( newtype::Vertex ) );
        memcpy( indcs.data(), txt_->mesh().indices_.data(), txt_->mesh().indices_.size() * sizeof( GLuint ) );
        mesh_->buffer().unlock();
        mesh_->indices().unlock();
      }
    }
  }

  void FontManager::draw( Renderer* renderer )
  {
    if ( mesh_ && fnt_ && fnt_->loaded() && !fnt_->dirty() )
    {
      auto& data = fontdata_[fnt_->id()];
      mesh_->draw( renderer->shaders(), data.material_->textureHandle( 0 ) );
    }
  }

  void FontManager::shutdown()
  {
    nt_.mgr()->unloadFont( fnt_ );
    fontdata_.clear();
    nt_.unload();
  }

}