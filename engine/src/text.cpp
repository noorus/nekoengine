#include "stdafx.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "text.h"

namespace neko {

  namespace features {

    const hb_tag_t KernTag = HB_TAG( 'k', 'e', 'r', 'n' ); // kerning operations
    const hb_tag_t LigaTag = HB_TAG( 'l', 'i', 'g', 'a' ); // standard ligature substitution
    const hb_tag_t CligTag = HB_TAG( 'c', 'l', 'i', 'g' ); // contextual ligature substitution

    static hb_feature_t LigatureOff = { LigaTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t LigatureOn = { LigaTag, 1, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t KerningOff = { KernTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t KerningOn = { KernTag, 1, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t CligOff = { CligTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t CligOn = { CligTag, 1, 0, std::numeric_limits<unsigned int>::max() };

  }

  Text::Text( ThreadedLoaderPtr loader, MeshManagerPtr meshmgr, FontPtr font ): dirty_( false ), font_( font )
  {
    mesh_ = meshmgr->createDynamic( gl::GL_TRIANGLES, VBO_3D, true, false );
    hbbuf_ = hb_buffer_create();
    language_ = "en";
    script_ = HB_SCRIPT_LATIN;
    direction_ = HB_DIRECTION_LTR;
  }

  Text::~Text()
  {
    hb_buffer_destroy( hbbuf_ );
  }

  void Text::set( const utf8String& text, vec2 pen )
  {
    text_ = unicodeString::fromUTF8( text );
    pen_ = pen;
    dirty_ = true;
    regenerate();
  }

  void Text::regenerate()
  {
    if ( !dirty_ || !font_->loaded() )
      return;

    hb_buffer_reset( hbbuf_ );
    hb_buffer_set_direction( hbbuf_, direction_ );
    hb_buffer_set_script( hbbuf_, script_ );
    hb_buffer_set_language( hbbuf_, hb_language_from_string( language_.c_str(), static_cast<int>( language_.size() ) ) );
    uint32_t flags = hb_buffer_get_flags( hbbuf_ );
    flags |= HB_BUFFER_FLAG_BOT;
    flags |= HB_BUFFER_FLAG_EOT;
    hb_buffer_set_flags( hbbuf_, static_cast<hb_buffer_flags_t>( flags ) );

    vector<hb_feature_t> feats = { features::KerningOn, features::LigatureOn, features::CligOn };

    hb_buffer_add_utf16( hbbuf_, reinterpret_cast<const uint16_t*>( text_.getBuffer() ), text_.length(), 0, text_.length() );
    hb_shape( font_->hbfnt_, hbbuf_, feats.empty() ? nullptr : feats.data(), static_cast<int>( feats.size() ) );

    vec2 position = pen_;

    const auto ascender = font_->ascender();
    const auto descender = font_->descender();

    position.y += ascender + descender;

    unsigned int glyphCount;
    auto info = hb_buffer_get_glyph_infos( hbbuf_, &glyphCount );
    auto gpos = hb_buffer_get_glyph_positions( hbbuf_, &glyphCount );
    for ( unsigned int i = 0; i < glyphCount; ++i )
    {
      auto codepoint = text_.charAt( i );
      auto glyphindex = info[i].codepoint;
      const auto chartype = u_charType( codepoint );
      // Locator::console().printf( Console::srcGfx, "index %i char %i glyph %i chartype %i cluster %i", i, codepoint, glyphindex, chartype, info[i].cluster );
      if ( chartype == U_CONTROL_CHAR && glyphindex == 0 )
      {
        position.x = pen_.x;
        position.y += ( font_->ascender() - font_->descender() + font_->linegap_ );
        continue;
      }
      auto glyph = font_->getGlyph( glyphindex );
      auto offset = vec2( gpos[i].x_offset, gpos[i].y_offset ) / 64.0f;
      auto advance = vec2( gpos[i].x_advance, gpos[i].y_advance ) / 64.0f;
      auto p0 = vec2(
        ( position.x + offset.x + glyph->bearing.x ),
        math::ifloor( position.y - offset.y - glyph->bearing.y ) );
      auto p1 = vec2(
        ( p0.x + glyph->width ),
        (int)( p0.y + glyph->height ) );
      auto index = (GLuint)mesh_->vertsCount();
      auto normal = vec3( 1.0f, 0.0f, 0.0f );
      auto color = vec4( 1.0f, 1.0f, 1.0f, 1.0f );
      vector<Vertex3D> vertices = {
        { vec3( p0.x, p0.y, 0.0f ), normal, vec2( glyph->coords[0].x, glyph->coords[0].y ), color },
        { vec3( p0.x, p1.y, 0.0f ), normal, vec2( glyph->coords[0].x, glyph->coords[1].y ), color },
        { vec3( p1.x, p1.y, 0.0f ), normal, vec2( glyph->coords[1].x, glyph->coords[1].y ), color },
        { vec3( p1.x, p0.y, 0.0f ), normal, vec2( glyph->coords[1].x, glyph->coords[0].y ), color } };
      vector<GLuint> indices = {
        index + 0, index + 1, index + 2, index + 0, index + 2, index + 3 };
      mesh_->pushIndices( move( indices ) );
      mesh_->pushVertices( move( vertices ) );
      position += advance;
    }

    dirty_ = false;
  }

  void Text::draw( Renderer* renderer )
  {
    regenerate();
    if ( dirty_ )
      return;

    mesh_->begin();
    font_->use( renderer, 0 );
    mesh_->draw();
    gl::glBindTextureUnit( 0, 0 );
  }

}