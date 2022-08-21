#include "pch.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "text.h"

namespace neko {

  Text::Text( newtype::Manager* manager, FontPtr font, newtype::StyleID style ):
  mgr_( manager ), font_( font )
  {
    text_ = mgr_->createText( font_->face(), style );
    // pen is an expensive call because it causes regeneration of the text mesh.
    // fortunately there's practically no need to ever use a non-zero origin pen
    // because translation can be done via the model matrix.
    text_->pen( vec3( 0.0f ) );
  }

  void Text::content( unicodeString text )
  {
    if ( text.compare( content_ ) == 0 )
      return;
    content_ = move( text );
    text_->text( content_ );
  }

  newtype::IDType Text::id() const
  {
    return text_->id();
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

  void Text::draw( Renderer* renderer )
  {
    const auto sid = text_->styleid();
    if ( mesh_ && font_ && font_->usable( sid ) )
    {
      mesh_->draw( renderer->shaders(),
        transform_.asModel4(),
        font_->style( sid ).material_->textureHandle( 0 )
      );
    }
  }

}