#include "pch.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "lodepng.h"
#include "renderer.h"
#include "engine.h"

namespace neko {

  void FontManager::initializeLogic()
  {
    nt_.load( this );

    engine_->console()->printf( Console::srcGfx, "Newtype reported: %s", nt_.mgr()->versionString().c_str() );

    fnt_ = make_shared<Font>( nt_.mgr() );
    ntfonts_[fnt_->id()] = fnt_;
    vector<uint8_t> input;
    Locator::fileSystem().openFile( Dir_Fonts, R"(SourceHanSansJP-Bold.otf)" )->readFullVector( input );
    fnt_->loadFace( input, 0, 24.0f );
    auto sid = fnt_->loadStyle( newtype::FontRender_Normal, 0.0f );

    createText( fnt_, sid );
  }

  Font::Font( newtype::Manager* manager ): mgr_( manager )
  {
    font_ = mgr_->createFont();
    font_->setUser( this );
  }

  void Font::loadFace( span<uint8_t> buffer, newtype::FaceID faceIndex, Real size )
  {
    assert( font_ );
    face_ = mgr_->loadFace( font_, buffer, faceIndex, size );
  }

  newtype::StyleID Font::loadStyle( newtype::FontRendering rendering, Real thickness )
  {
    assert( face_ );
    auto sid = mgr_->loadStyle( face_, rendering, thickness );
    styles_[sid].id_ = sid;
    return sid;
  }

  newtype::IDType Font::id() const
  {
    return font_->id();
  }

  Font::~Font()
  {
    // Make sure to free the face ref so it gets destroyed before the font
    // ...hopefully
    face_.reset();
    mgr_->unloadFont( font_ );
    font_.reset();
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

  void FontManager::shutdownRender( Renderer* renderer )
  {
    txts_.clear();
    ntfonts_.clear();
  }

  void FontManager::shutdownLogic()
  {
    nt_.unload();
  }

}