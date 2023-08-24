#include "pch.h"
#include "components.h"
#include "camera.h"
#include "utilities.h"
#include "spriteanim.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  namespace c {

    sprite_system::sprite_system( manager* m ): mgr_( m )
    {
      mgr_->reg().on_construct<sprite>().connect<&sprite_system::addSprite>( this );
      mgr_->reg().on_update<sprite>().connect<&sprite_system::updateSprite>( this );
      mgr_->reg().on_destroy<sprite>().connect<&sprite_system::removeSprite>( this );
    }

    sprite_system::~sprite_system()
    {
      mgr_->reg().on_construct<sprite>().disconnect<&sprite_system::addSprite>( this );
      mgr_->reg().on_update<sprite>().disconnect<&sprite_system::updateSprite>( this );
      mgr_->reg().on_destroy<sprite>().disconnect<&sprite_system::removeSprite>( this );
    }

    void sprite_system::addSprite( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_sprite>( e );
    }

    void sprite_system::updateSprite( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_sprite>( e );
    }

    void sprite_system::removeSprite( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_sprite>( e );
    }

    void sprite_system::update( MaterialManager& mats )
    {
      set<entity> bad;

      auto view = mgr_->reg().view<dirty_sprite>();
      for ( auto e : view )
      {
        auto& s = mgr_->s( e );
        if ( !s.material )
        {
          s.material = mats.getPtr( s.matName );
        }
        if ( s.size < 0.00001f || !s.material || !s.material->uploaded() )
        {
          s.mesh.reset();
          bad.insert( e );
        }
        else
        {
          s.dimensions = { static_cast<Real>( s.material->width() ), static_cast<Real>( s.material->height() ) };
          auto parts =
            Locator::meshGenerator().makePlane(
              s.dimensions * c_pixelScaleValues[s.pixelScaleBase],
              { 1, 1 },
              { 0.0f, 0.0f, 1.0f } );
          if ( !s.mesh || s.mesh->buffer().size() != parts.first.size() ||
                s.mesh->indices().size() != parts.second.size() )
            s.mesh = make_unique<SpriteVertexbuffer>( parts.first.size(), parts.second.size() );
          const auto& verts = s.mesh->buffer().lock();
          const auto& indces = s.mesh->indices().lock();
          memcpy( verts.data(), parts.first.data(), parts.first.size() * sizeof( Vertex3D ) );
          memcpy( indces.data(), parts.second.data(), parts.second.size() * sizeof( GLuint ) );
          s.mesh->buffer().unlock();
          s.mesh->indices().unlock();
        }
      }

      mgr_->reg().clear<dirty_sprite>();
      for ( auto& e : bad )
        mgr_->reg().emplace_or_replace<dirty_sprite>( e );
    }

    void sprite_system::draw( Renderer& renderer, const Camera& cam )
    {
      auto view = mgr_->reg().view<sprite>();
      for ( auto e : view )
      {
        auto& s = mgr_->s( e );

        if ( !s.mesh || !s.material || !s.material->uploaded() )
          continue;

        auto& t = mgr_->tn( e );

        auto model = s.billboard ? t.model() * mat4( mat3( math::transpose( math::inverse( cam.view() ) ) ) ) : t.model();
        s.mesh->draw( renderer.shaders(), *s.material, s.frame, model );
      }
    }

    void sprite_system::imguiSpriteEditor( entity e )
    {
      ig::ComponentChildWrapper wrap( "Sprite", 200.0f );

      auto& s = mgr_->reg().get<sprite>( e );

      bool changed = false;

      auto matchanged = ig::imguiInputText( "material", &s.matName, false, nullptr, &s.int_ud_ );
      changed |= ig::imguiPixelScaleSelector( s.pixelScaleBase );
      changed |= ImGui::SliderInt( "frame", &s.frame, 0, 32 );
      ImGui::Checkbox( "billboard", &s.billboard );
      if ( matchanged )
        s.material.reset();
      if ( changed || matchanged )
        mgr_->reg().emplace_or_replace<dirty_sprite>( e );
    }

  }

}