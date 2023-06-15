#include "pch.h"
#include "components.h"
#include "camera.h"
#include "utilities.h"
#include "font.h"

namespace neko {

  namespace c {

    primitive_system::primitive_system( manager* m ): mgr_( m )
    {
      mgr_->reg().on_construct<primitive>().connect<&primitive_system::addPrimitive>( this );
      mgr_->reg().on_update<primitive>().connect<&primitive_system::updatePrimitive>( this );
      mgr_->reg().on_destroy<primitive>().connect<&primitive_system::removePrimitive>( this );
    }

    primitive_system::~primitive_system()
    {
      mgr_->reg().on_construct<primitive>().disconnect<&primitive_system::addPrimitive>( this );
      mgr_->reg().on_update<primitive>().disconnect<&primitive_system::updatePrimitive>( this );
      mgr_->reg().on_destroy<primitive>().disconnect<&primitive_system::removePrimitive>( this );
    }

    void primitive_system::addPrimitive( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_primitive>( e );
    }

    void primitive_system::updatePrimitive( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_primitive>( e );
    }

    void primitive_system::removePrimitive( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_primitive>( e );
    }

    void primitive_system::update()
    {
      auto view = mgr_->reg().view<dirty_primitive>();
      for ( auto entity : view )
      {
        auto& p = mgr_->pt( entity );
        if ( p.type == primitive::PrimitiveType::Plane )
        {
          auto& vals = p.values.plane;
          vals.normal = ig::valueForNormalIndex( vals.normal_sel );
          if ( vals.segments.x < 1 || vals.segments.y < 1 || vals.dimensions.x < 0.01f || vals.dimensions.y < 0.01f )
            p.mesh.reset();
          else
          {
            auto parts = Locator::meshGenerator().makePlane( vals.dimensions, vals.segments, vals.normal );
            if ( !p.mesh || p.mesh->buffer().size() != parts.first.size() ||
                 p.mesh->indices().size() != parts.second.size() )
              p.mesh = make_unique<BasicIndexedVertexbuffer>( parts.first.size(), parts.second.size() );
            const auto& verts = p.mesh->buffer().lock();
            const auto& indces = p.mesh->indices().lock();
            memcpy( verts.data(), parts.first.data(), parts.first.size() * sizeof( Vertex3D ) );
            memcpy( indces.data(), parts.second.data(), parts.second.size() * sizeof( GLuint ) );
            p.mesh->buffer().unlock();
            p.mesh->indices().unlock();
            //mesh.vbo_->pushVertices( retPair.first );
            //mesh.ebo_->storage_.insert( mesh.ebo_->storage_.end(), retPair.second.begin(), retPair.second.end() );
          }
        }
      }

      mgr_->reg().clear<dirty_primitive>();
    }

    void primitive_system::draw( shaders::Shaders& shaders, const Material& mat )
    {
      auto view = mgr_->reg().view<primitive>();
      for ( auto entity : view )
      {
        auto& p = mgr_->pt( entity );
        if ( !p.mesh )
          continue;
        auto& t = mgr_->tn( entity );
        
        p.mesh->draw( shaders, mat, t.model() );
      }
    }

    void primitive_system::imguiPrimitiveEditor( entity e )
    {
      bool changed = false;

      auto& c = mgr_->reg().get<primitive>( e );
      if ( c.type == primitive::PrimitiveType::Plane )
      {
        ig::ComponentChildWrapper wrap( "Primitive", 200.0f );
        changed |= ig::dragVector( "dimensions", c.values.plane.dimensions, 0.1f, 0.0f, 0.0f, "%.4f" );
        changed |= ImGui::DragInt2( "segments", &c.values.plane.segments[0], 1.0f, 1, 100 );
        changed |= ig::normalSelector( "normal", c.values.plane.normal_sel, c.values.plane.normal );
      }

      if ( changed )
        mgr_->reg().emplace_or_replace<dirty_primitive>( e );
    }

  }

}