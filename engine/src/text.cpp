#include "pch.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "renderer.h"

namespace neko {

  // clang-format off

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

  Text::Text( FontManagerPtr manager, IDType id, FontStylePtr style, const Text::Features& features ):
  manager_( manager ), id_( id ), style_( style )
  {
    hbbuf_ = make_unique<HBBuffer>( "en" );

    features_.push_back( features.kerning ? features::KerningOn : features::KerningOff );
    features_.push_back( features.ligatures ? features::LigatureOn : features::LigatureOff );
    features_.push_back( features.ligatures ? features::CligOn : features::CligOff );
  }

  void Text::text( const unicodeString& text )
  {
    if ( text.compare( text_ ) == 0 || text_ == text )
      return;

    text_ = text;
    dirty_ = true;
  }

  const unicodeString& Text::text()
  {
    return text_;
  }

  void Text::style( FontStylePtr newStyle )
  {
    if ( style_ == newStyle )
      return;

    style_ = newStyle;
    dirty_ = true;
  }

  void Text::regenerate()
  {
    if ( !dirty_ || !style_ || !style_->face()->font()->loaded() )
      return;
    
    // TODO handle special case where textdata doesn't exist (= generate empty mesh)

    hbbuf_->setFrom( style_->hbfnt_, features_, text_ );

    const auto ascender = style_->ascender();
    const auto descender = style_->descender();
    const auto lineheight = ( ascender - descender );

    vertices_.clear();
    indices_.clear();

    vec2 minpos { std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max() };
    meshDimensions_ = { 0.0f, 0.0f };

    // TODO figure out the actual properties to use for offsetting, not this * 1.6f hardcoded nonsense
    vec3 position( 0.0f, lineheight * 1.6f, 0.0f );

    for ( unsigned int i = 0; i < hbbuf_->count(); ++i )
    {
      auto codepoint = text_.charAt( i );
      auto glyphindex = hbbuf_->glyphInfo()[i].codepoint;
      const auto& gpos = hbbuf_->glyphPosition()[i];

      const auto chartype = u_charType( codepoint );
      if ( chartype == U_CONTROL_CHAR && glyphindex == 0 )
      {
        position.x = 0.0f;
        position.y += lineheight;
        continue;
      }

      auto glyph = style_->getGlyph( manager_->ft(), style_->face_->face_, glyphindex );
      auto offset = vec2( gpos.x_offset, gpos.y_offset ) / c_fmagic;
      auto advance = ( vec2( gpos.x_advance, gpos.y_advance ) / c_fmagic );

      // bearing = bitmap_left/bitmap_top
      auto p0 = vec2(
        ( position.x + offset.x + glyph->bearing.x ),
        ( position.y - offset.y - glyph->bearing.y ) );

      auto p1 = vec2(
        ( p0.x + glyph->width ),
        ( p0.y + glyph->height ) );

      auto color = vec4( 1.0f, 1.0f, 1.0f, 1.0f );

      minpos.x = math::min( p0.x, minpos.x );
      minpos.y = math::min( p0.y, minpos.y );
      meshDimensions_.x = math::max( p1.x, meshDimensions_.x );
      meshDimensions_.y = math::max( p1.y, meshDimensions_.y );

      auto index = static_cast<VertexIndex>( vertices_.size() );
      vertices_.emplace_back( vec3( p0.x, lineheight - p0.y, position.z ), vec2( glyph->coords[0].x, glyph->coords[0].y ), color );
      vertices_.emplace_back( vec3( p0.x, lineheight - p1.y, position.z ), vec2( glyph->coords[0].x, glyph->coords[1].y ), color );
      vertices_.emplace_back( vec3( p1.x, lineheight - p1.y, position.z ), vec2( glyph->coords[1].x, glyph->coords[1].y ), color );
      vertices_.emplace_back( vec3( p1.x, lineheight - p0.y, position.z ), vec2( glyph->coords[1].x, glyph->coords[0].y ), color );

      Indices idcs = { index + 0, index + 1, index + 2, index + 0, index + 2, index + 3 };
      indices_.insert( indices_.end(), idcs.begin(), idcs.end() );

      position += vec3( advance, 0.0f );
    }

    meshDimensions_ -= minpos;

    dirty_ = false;
  }

  void Text::update( Renderer& renderer )
  {
    bool wasDirty = dirty_;
    regenerate();
   
    if ( wasDirty && mesh_ )
      mesh_.reset();
    if ( !mesh_ && !vertices_.empty() && !indices_.empty() )
    {
      mesh_ = make_unique<TextRenderBuffer>(
        static_cast<gl::GLuint>( vertices_.size() ),
        static_cast<gl::GLuint>( indices_.size() ) );
      const auto& verts = mesh_->buffer().lock();
      const auto& indcs = mesh_->indices().lock();
      memcpy( verts.data(), vertices_.data(), vertices_.size() * sizeof( VertexText ) );
      memcpy( indcs.data(), indices_.data(), indices_.size() * sizeof( GLuint ) );
      mesh_->buffer().unlock();
      mesh_->indices().unlock();
    }
  }

  void Text::draw( Renderer& renderer, const mat4& modelMatrix )
  {
    if ( !mesh_ || !style_ )
      return;
    if ( style_ && style_->material_ )
    {
      mesh_->draw( renderer.shaders(), modelMatrix,
        style_->material_->textureHandle( 0 )
      );
    }
  }

}