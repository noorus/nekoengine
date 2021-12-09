#include "stdafx.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "text.h"

namespace neko {

  Text::Text( ThreadedLoaderPtr loader, MeshManagerPtr meshmgr, FontPtr font ): dirty_( false ), font_( font )
  {
    mesh_ = meshmgr->createDynamic( gl::GL_TRIANGLES, VBO_3D, true, false );
    hbbuf_ = hb_buffer_create();
  }

  Text::~Text()
  {
    hb_buffer_destroy( hbbuf_ );
  }

  void Text::set( const utf8String& text, vec2 pen )
  {
    text_ = text;
    pen_ = pen;
    dirty_ = true;
    regenerate();
  }

  /* void Text::hbgenerate()
  {
    hb_buffer_reset( hbbuf_ );
    hb_buffer_set_direction( hbbuf_, direction_ );
    hb_buffer_set_script( hbbuf_, script_ );
    hb_buffer_set_language( hbbuf_, hb_language_from_string( language_.c_str(), language_.size() ) );

    hb_buffer_add_utf8( hbbuf_, text_.c_str(), text_.size(), 0, text_.size() );
    hb_shape( font_->hbfnt_, 0, 0 );
  }*/

  void Text::regenerate()
  {
    if ( !dirty_ || !font_->loaded() )
      return;

    uint32_t previousCodepoint = 0;
    vec2 position = pen_;

    const auto ascender = font_->ascender();
    const auto descender = font_->descender();

    position.y += ascender + descender;
    for ( size_t i = 0; i < text_.length(); ++i )
    {
      if ( text_[i] == '\n' )
      {
        position.x = pen_.x;
        position.y += ascender; // font_->impl_->descender_ + font_->impl_->ascender_ + 2.0f;
        previousCodepoint = 0;
        continue;
      }
      auto codepoint = utils::utf8_to_utf32( &text_[i] );
      auto glyph = font_->getGlyph( codepoint );
      Real kerning = ( previousCodepoint > 0 ? glyph->getKerning( previousCodepoint ) : 0.0f );
      previousCodepoint = codepoint;
      position.x -= kerning;
      auto p0 = vec2(
        ( position.x + glyph->offset.x ),
        (int)( position.y - glyph->offset.y ) );
      auto p1 = vec2(
        ( p0.x + glyph->width ),
        (int)( p0.y + glyph->height ) );
      auto index = (GLuint)mesh_->vertsCount();
      auto normal = vec3( 1.0f, 0.0f, 0.0f );
      auto color = vec4( 0.0f, 1.0f, 1.0f, 1.0f );
      vector<Vertex3D> vertices = {
        { vec3( p0.x, p0.y, 0.0f ), normal, vec2( glyph->coords[0].x, glyph->coords[0].y ), color },
        { vec3( p0.x, p1.y, 0.0f ), normal, vec2( glyph->coords[0].x, glyph->coords[1].y ), color },
        { vec3( p1.x, p1.y, 0.0f ), normal, vec2( glyph->coords[1].x, glyph->coords[1].y ), color },
        { vec3( p1.x, p0.y, 0.0f ), normal, vec2( glyph->coords[1].x, glyph->coords[0].y ), color } };
      vector<GLuint> indices = {
        index + 0, index + 1, index + 2, index + 0, index + 2, index + 3 };
      mesh_->pushIndices( move( indices ) );
      mesh_->pushVertices( move( vertices ) );
      position.x += glyph->advance.x;
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