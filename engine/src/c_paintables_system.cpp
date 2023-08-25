#include "pch.h"
#include "components.h"
#include "camera.h"
#include "utilities.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  namespace c {

    paintables_system::paintables_system( manager* m ): mgr_( m )
    {
      mgr_->reg().on_construct<paintable>().connect<&paintables_system::addSurface>( this );
      mgr_->reg().on_update<paintable>().connect<&paintables_system::updateSurface>( this );
      mgr_->reg().on_destroy<paintable>().connect<&paintables_system::removeSurface>( this );
    }

    paintables_system::~paintables_system()
    {
      mgr_->reg().on_construct<paintable>().disconnect<&paintables_system::addSurface>( this );
      mgr_->reg().on_update<paintable>().disconnect<&paintables_system::updateSurface>( this );
      mgr_->reg().on_destroy<paintable>().disconnect<&paintables_system::removeSurface>( this );
    }

    void paintables_system::addSurface( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_paintable>( e );
    }

    void paintables_system::updateSurface( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_paintable>( e );
    }

    void paintables_system::removeSurface( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_paintable>( e );
    }

    void paintable::mouseClickTest(
      manager* m, entity e, Renderer& renderer, const Ray& ray, const vec2i& mousepos, int button )
    {
      bool hit = false;
      vec2 hitpos;
      if ( mesh && mesh->indices().size() == 6 )
      {
        auto& t = m->tn( e );
        const auto& verts = mesh->buffer().lock();
        const auto& indices = mesh->indices().lock();
        auto reverse = false;
        for ( int i = 0; i < 2; ++i )
        {
          // this will definitely not work for any other purpose
          auto triangle = ( i * 3 );
          vec3 v[3];
          if ( reverse )
          {
            v[0] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 0]].position, 1.0f ) );
            v[1] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 1]].position, 1.0f ) );
            v[2] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 2]].position, 1.0f ) );
            hit = math::rayTriangleIntersection( ray, v[0], v[1], v[2], hitpos.y, hitpos.x );
          }
          else
          {
            v[0] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 1]].position, 1.0f ) );
            v[1] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 2]].position, 1.0f ) );
            v[2] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 0]].position, 1.0f ) );
            hit = math::rayTriangleIntersection( ray, v[0], v[1], v[2], hitpos.y, hitpos.x );
            hitpos.x = ( 1.0f - hitpos.x );
            hitpos.y = ( 1.0f - hitpos.y );
          }
          reverse = ( !reverse );
          if ( hit )
            break;
        }
        mesh->buffer().unlock();
        mesh->indices().unlock();
      }
      if ( hit && lastPaintPos != mousepos )
      {
        lastPaintPos = mousepos;
        applyPaint( renderer, hitpos );
      }
    }

    void paintable::applyPaint( Renderer& renderer, const vec2& pos )
    {
      if ( !texture )
        return;

      auto& pipeline = renderer.shaders().usePipeline( "tool_2dpaint" );
      auto pxpos = vec2i(
        math::min( math::max( math::iround( pos.x * static_cast<Real>( texture->width() ) ), 0 ), texture->width() - 1 ),
        math::min( math::max( math::iround( pos.y * static_cast<Real>( texture->height() ) ), 0 ), texture->height() - 1 )
      );
      pipeline.setUniform( "paint_mousepos", pxpos );
      pipeline.setUniform( "paint_brushsize", paintBrushSize );
      pipeline.setUniform( "paint_brushsoften", paintBrushSoftness );
      pipeline.setUniform( "paint_brushopacity", paintBrushOpacity );
      auto clr = vec4( 0.0f, 0.7f, 1.0f, 1.0f );
      pipeline.setUniform( "paint_brushcolor", clr );
      renderer.bindImageTexture( 0, texture->texture() );
      renderer.bindTexture( 0, texture->texture() );
      glDispatchCompute( texture->width() / 8, texture->height() / 8, 1 );
      glMemoryBarrier( gl::MemoryBarrierMask::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
    }

    void paintables_system::update( Renderer& renderer )
    {
      set<entity> bad;

      auto view = mgr_->reg().view<dirty_paintable>();
      for ( auto e : view )
      {
        auto& pt = mgr_->paintable2d( e );
        auto parts =
          Locator::meshGenerator().makePlane(
            vec2( pt.dimensions ) * c_pixelScaleValues[pt.pixelScaleBase],
            { 1, 1 },
            { 0.0f, 0.0f, 1.0f } );
        if ( !pt.mesh || pt.mesh->buffer().size() != parts.first.size() ||
              pt.mesh->indices().size() != parts.second.size() )
          pt.mesh = make_unique<BasicIndexedVertexbuffer>( parts.first.size(), parts.second.size() );
        const auto& verts = pt.mesh->buffer().lock();
        const auto& indces = pt.mesh->indices().lock();
        memcpy( verts.data(), parts.first.data(), parts.first.size() * sizeof( Vertex3D ) );
        memcpy( indces.data(), parts.second.data(), parts.second.size() * sizeof( GLuint ) );
        pt.mesh->buffer().unlock();
        pt.mesh->indices().unlock();
        if ( !pt.texture || pt.texture->width() != pt.dimensions.x || pt.texture->height() != pt.dimensions.y )
          pt.texture = make_shared<PaintableTexture>( renderer, pt.dimensions.x, pt.dimensions.y, PixFmtColorRGBA32f );
      }

      mgr_->reg().clear<dirty_paintable>();
      for ( auto& e : bad )
        mgr_->reg().emplace_or_replace<dirty_paintable>( e );
    }

    void paintables_system::draw( Renderer& renderer, const Camera& cam )
    {
      auto view = mgr_->reg().view<paintable>();
      for ( auto e : view )
      {
        auto& pt = mgr_->paintable2d( e );
        if ( !pt.mesh || !pt.texture )
          continue;
        auto& t = mgr_->tn( e );
        pt.mesh->draw( renderer.shaders(), pt.texture->handle(), t.model() );
      }
    }

    void paintables_system::imguiPaintableSurfaceEditor( entity e )
    {
      ig::ComponentChildWrapper wrap( "Paintable", 200.0f );

      auto& pt = mgr_->reg().get<paintable>( e );

      bool changed = false;
      changed |= ig::imguiPixelScaleSelector( pt.pixelScaleBase );
      changed |= ImGui::DragInt2( "dimensions", &pt.dimensions[0], 1.0f, 0, 4096 );
      changed |= ig::normalSelector( "normal", pt.normal_sel, pt.normal );
      ig::ComponentChildWrapper wrap( "Brush", 200.0f );
      changed |= ImGui::SliderInt( "brush size", &pt.paintBrushSize, 1, 100 );
      changed |= ImGui::SliderFloat( "brush softness", &pt.paintBrushSoftness, 0.0f, 1.0f );
      changed |= ImGui::SliderFloat( "brush opacity", &pt.paintBrushOpacity, 0.0f, 1.0f );
      if ( changed )
        mgr_->reg().emplace_or_replace<dirty_paintable>( e );
    }

  }

}