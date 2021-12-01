#include "stdafx.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "text.h"

namespace neko {

  DynamicText::DynamicText( ThreadedLoaderPtr loader, MeshManagerPtr meshmgr, FontManagerPtr fontmgr ): meshGenerated_( false )
  {
    font_ = fontmgr->createFont();
    loader->addLoadTask( { LoadTask( font_, R"(SourceHanSansJP-Bold.otf)", 24.0f ) } );
    mesh_ = meshmgr->createDynamic( gl::GL_TRIANGLES, VBO_3D, true, false );
  }

  DynamicText::~DynamicText()
  {
  }

  void DynamicText::setText( const utf8String& text, vec2 pen )
  {
    text_ = text;
    pen_ = pen;
    meshGenerated_ = false;
    regenerate();
  }

  void DynamicText::regenerate()
  {
    if ( meshGenerated_ || !font_->loaded_ )
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
        continue;
      }
      auto codepoint = utils::utf8_to_utf32( &text_[i] );
      auto glyph = font_->impl_->getGlyph( codepoint );
      Real kerning = ( i > 0 ? glyph->getKerning( previousCodepoint ) : 0.0f );
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
    meshGenerated_ = true;
  }

  void DynamicText::draw( Renderer* renderer )
  {
    regenerate();
    if ( !meshGenerated_ )
      return;
    if ( !material_ )
    {
      material_ = renderer->createTextureWithData( "font", font_->impl_->atlas_->width_, font_->impl_->atlas_->height_, PixFmtColorR8, (const void*)font_->impl_->atlas_->data_.data(), Texture::ClampBorder, Texture::Nearest );
      /*platform::FileWriter writer( "debug.png" );
        vector<uint8_t> buffer;
        lodepng::encode( buffer, font_->impl_->atlas_->data_, font_->impl_->atlas_->width_, font_->impl_->atlas_->height_, LCT_GREY, 8 );
        writer.writeBlob( buffer.data(), buffer.size() );*/
    }

    mesh_->begin();
    gl::glBindTextureUnit( 0, material_->layers_[0].texture_->handle() );
    mesh_->draw();
    gl::glBindTextureUnit( 0, 0 );
  }

}