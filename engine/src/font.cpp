#include "pch.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "lodepng.h"
#include "renderer.h"
#include "engine.h"

namespace neko {

  const static unicodeString g_prerenderGlyphs = utils::uniFrom( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );

  Font::Font( newtype::Manager* manager, const utf8String& name ):
  LoadedResourceBase<Font>( name ), mgr_( manager )
  {
    font_ = mgr_->createFont();
    font_->user( this );
  }

  void Font::loadFace( span<uint8_t> buffer, newtype::FaceID faceIndex, Real size )
  {
    assert( font_ );
    face_ = mgr_->loadFace( font_, buffer, faceIndex, size );
  }

  newtype::StyleID Font::loadStyle( newtype::FontRendering rendering, Real thickness )
  {
    assert( face_ );
    auto sid = mgr_->loadStyle( face_, rendering, thickness, g_prerenderGlyphs );
    styles_[sid].id_ = sid;
    Locator::console().printf(
      Console::srcEngine, "Font %s has style 0x%x (%i, thickness %.2f)", name_.c_str(), sid, rendering, thickness );
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
    return ( !face_->style( style )->dirty() );
  }

  void Font::update( Renderer* renderer )
  {
    if ( !font_->loaded() )
      return;
    for ( auto& entry : styles_ )
    {
      auto styleId = entry.first;
      auto style = face_->style( styleId );
      if ( !style->dirty() )
        continue;
      char tmp[64];
      sprintf_s( tmp, 64, "font/%I64i/%016I64x", font_->id(), styleId );
      entry.second.material_ = renderer->createTextureWithData( tmp,
        style->texture().dimensions().x, style->texture().dimensions().y,
        PixFmtColorR8, style->texture().data(),
        Texture::ClampBorder, Texture::Nearest );
      /* dump texture here */
      sprintf_s( tmp, 64, "debug_fontatlas_%I64i_%016I64x.png", font_->id(), styleId );
      platform::FileWriter writer( tmp );
      vector<uint8_t> buffer;
      lodepng::encode( buffer, style->texture().data(), static_cast<unsigned int>( style->texture().dimensions().x ),
        static_cast<unsigned int>( style->texture().dimensions().y ),
        LCT_GREY, 8 );
      writer.writeBlob( buffer.data(), static_cast<uint32_t>( buffer.size() ) );
      style->markClean();
    }
  }

}