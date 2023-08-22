#include "pch.h"
#include "components.h"
#include "camera.h"
#include "utilities.h"
#include "font.h"

namespace neko {

  using namespace gl;

  namespace c {

    text_system::text_system( manager* m ): mgr_( m )
    {
      mgr_->reg().on_construct<text>().connect<&text_system::addText>( this );
      mgr_->reg().on_update<text>().connect<&text_system::updateText>( this );
      mgr_->reg().on_destroy<text>().connect<&text_system::removeText>( this );
    }

    text_system::~text_system()
    {
      mgr_->reg().on_construct<text>().disconnect<&text_system::addText>( this );
      mgr_->reg().on_update<text>().disconnect<&text_system::updateText>( this );
      mgr_->reg().on_destroy<text>().disconnect<&text_system::removeText>( this );
    }

    void text_system::addText( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_text>( e );
    }

    void text_system::updateText( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_text>( e );
    }

    void text_system::removeText( registry& r, entity e )
    {
      if ( texts_.find( e ) != texts_.end() )
      {
        if ( texts_[e].instance )
          texts_[e].instance->markDead();
        texts_.erase( e );
      }
    }

    void text_system::draw( Renderer& renderer )
    {
      glDisable( GL_LINE_SMOOTH );
      glDisable( GL_POLYGON_SMOOTH );
      for ( auto& [eid, data] : texts_ )
      {
        if ( !data.instance || data.instance->dead() )
          continue;
        const auto& tfm = mgr_->tn( eid );
        const auto& t = mgr_->tt( eid );
        auto model = tfm.model() * glm::scale( vec3( t.size / data.instance->style()->size() ) );
        data.instance->draw( renderer, model );
      }
    }

    void text_system::update( FontManager& fntmgr )
    {
      mgr_->reg().view<dirty_text>().each( [this]( const auto entity )
      {
        const auto& t = mgr_->tt( entity );
        if ( texts_.find( entity ) == texts_.end() )
          texts_[entity] = TextData( entity );
        auto& te = texts_[entity];
        te.dirty = true;
      } );

      mgr_->reg().clear<dirty_text>();

      for ( auto& [eid, data] : texts_ )
      {
        if ( !data.dirty || ( data.instance && data.instance->dead() ) )
          continue;

        const auto& t = mgr_->tt( eid );

        auto fnt = fntmgr.font( t.fontName );
        if ( !fnt || !fnt->loaded() )
        {
          data.instance.reset();
          continue;
        }

        Real bestDelta = 0.0f;
        FontStylePtr bestStyle;
        for ( const auto& [fid, face] : fnt->faces() )
        {
          for ( const auto& [sid, style] : face->styles() )
          {
            auto delta = ( style->size() - t.size );
            if ( delta < 0.0f )
              delta = math::abs( delta ) * 0.5f;
            if ( !bestStyle || delta < bestDelta )
            {
              bestStyle = style;
              bestDelta = delta;
            }
          }
        }

        if ( !bestStyle )
          continue;

        if ( !data.instance )
          data.instance = fntmgr.createText( bestStyle );
        else
          data.instance->style( bestStyle );

        data.instance->content( utils::uniFrom( t.content ) );

        data.dirty = false;
      }
    }

    void text_system::imguiTextEditor( entity e )
    {
      ig::ComponentChildWrapper wrap( "Text", 200.0f );

      auto& c = mgr_->reg().get<text>( e );

      bool changed = false;
      changed |= ig::imguiInputText( "fontname", &c.fontName, false, nullptr, &c.int_ud_ );
      changed |= ImGui::SliderFloat( "size", &c.size, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp );
      changed |= ig::imguiInputText( "content", &c.content, true, nullptr, &c.int_ud_ );

      if ( changed )
        mgr_->reg().emplace_or_replace<dirty_text>( e );
    }

  }

}