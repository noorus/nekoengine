#include "stdafx.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "lodepng.h"
#include "renderer.h"
#include "engine.h"

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

    engine_->console()->printf( Console::srcGfx, "Newtype reported: %s", nt_.mgr()->versionString().c_str() );

    fnt_ = make_shared<Font>();
    fnt_->font_ = nt_.mgr()->createFont();
    ntfonts_[fnt_->font_->id()] = fnt_;
    fnt_->font_->setUser( fnt_.get() );
    vector<uint8_t> input;
    Locator::fileSystem().openFile( Dir_Fonts, R"(SourceHanSansJP-Bold.otf)" )->readFullVector( input );
    fnt_->face_ = nt_.mgr()->loadFace( fnt_->font_, input, 0, 24.0f );
    auto sid = nt_.mgr()->loadStyle( fnt_->face_, newtype::FontRender_Normal, 0.0f );
    fnt_->styles_[sid].id_ = sid;

    createText( fnt_, sid );
  }

  TextPtr FontManager::createText( FontPtr font, newtype::StyleID style )
  {
    auto txt = make_shared<Text>();
    txt->font_ = font;
    txt->text_ = nt_.mgr()->createText( font->face_, style );
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

  bool Font::usable( newtype::StyleID style ) const
  {
    if ( !font_ || !font_->loaded() )
      return false;
    if ( !face_ || styles_.find( style ) == styles_.end() )
      return false;
    return ( !face_->getStyle( style )->dirty() );
  }

  void Font::update( Renderer* renderer )
  {
    if ( !font_->loaded() )
      return;
    for ( auto& entry : styles_ )
    {
      auto styleId = entry.first;
      auto style = face_->getStyle( styleId );
      if ( !style->dirty() )
        continue;
      char tmp[64];
      sprintf_s( tmp, 64, "font/%I64i/%016I64x", font_->id(), styleId );
      entry.second.material_ = renderer->createTextureWithData( tmp,
        style->texture().dimensions().x, style->texture().dimensions().y,
        PixFmtColorR8, style->texture().data(),
        Texture::ClampBorder, Texture::Nearest );
      /* dump texture here
      platform::FileWriter writer( "fonttest_atlas.png" );
      vector<uint8_t> buffer;
      lodepng::encode( buffer,
        font->texture().data(),
        static_cast<unsigned int>( font->texture().dimensions().x ),
        static_cast<unsigned int>( font->texture().dimensions().y ),
        LCT_GREY, 8 );
      writer.writeBlob( buffer.data(), static_cast<uint32_t>( buffer.size() ) ); */
      style->markClean();
    }
  }

  void Text::update( Renderer* renderer )
  {
    text_->update();
    if ( text_->dirty() && mesh_ )
      mesh_.reset();
    if ( !text_->dirty() && !mesh_ )
    {
      mesh_ = make_unique<TextRenderBuffer>(
        static_cast<gl::GLuint>( text_->mesh().vertices_.size() ),
        static_cast<gl::GLuint>( text_->mesh().indices_.size() ) );
      const auto& verts = mesh_->buffer().lock();
      const auto& indcs = mesh_->indices().lock();
      memcpy( verts.data(), text_->mesh().vertices_.data(), text_->mesh().vertices_.size() * sizeof( newtype::Vertex ) );
      memcpy( indcs.data(), text_->mesh().indices_.data(), text_->mesh().indices_.size() * sizeof( GLuint ) );
      mesh_->buffer().unlock();
      mesh_->indices().unlock();
    }
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
          txt->font_->styles_[sid].material_->textureHandle( 0 )
        );
      }
    }
  }

  void FontManager::shutdown()
  {
    txts_.clear();
    for ( const auto& fnt : ntfonts_ )
    {
      nt_.mgr()->unloadFont( fnt.second->font_ );
      fnt.second->font_.reset();
    }
    nt_.unload();
  }

}