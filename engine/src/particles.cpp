#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"
#include "meshmanager.h"
#include "shaders.h"
#include "camera.h"
#include "loader.h"
#include "engine.h"
#include "lodepng.h"
#include "gfx.h"
#include "nekosimd.h"
#include "particles.h"

namespace neko {

  using namespace gl;

  SakuraSystem::SakuraSystem( const aabb& box ): Base(), box_( box )
  {
    resetSystem();
    boxviz_ = make_unique<LineRenderBuffer>();
  }

  void SakuraSystem::resetParticle( size_t index )
  {
    axes_[index] = glm::normalize( vec3( math::rand(), math::rand(), math::rand() ) );
    rot_[index] = math::rand() * 360.0f;
    positions_[index] = vec4(
      box_.x() + ( math::rand() * box_.width() ),
      box_.y() + ( math::rand() * box_.height() ),
      box_.max().z,
      0.0f );
    acceleration_[index] = vec4( 0.0f );
    velocities_[index] = vec4(
      0.0f,
      -( 0.045f + ( math::rand() * 0.02f ) ),
      -( 0.02f + ( math::rand() * 0.015f ) ),
      0.0f );
    masses_[index] = 0.000001f + ( math::rand() * 0.00005f );
    colors_[index] = vec4( 1.0f );
    sizes_[index] = vec3( 0.2f + math::rand() * 0.1f );
  }

  void SakuraSystem::update( GameTime delta )
  {
    for ( size_t i = 0; i < c_particleCount; ++i )
    {
      if ( !box_.intersects( positions_[i] ) )
      {
        resetParticle( i );
        continue;
      }
      rot_[i] += static_cast<float>( delta * 100.0 );
      orientations_[i] = math::angleAxis( math::radians( rot_[i] ), axes_[i] );
    }
    Base::update( delta );
  }

  void SakuraSystem::draw( shaders::Shaders& shaders, const Material& mat )
  {
    Base::draw( shaders, mat );
    boxviz_->buffer().lock();
    auto verts = boxviz_->buffer().buffer().data();
    const auto color = vec4( 0.0f, 1.0f, 0.0f, 1.0f );
    const auto mi = box_.min();
    const auto ma = box_.max();
    for ( size_t i = 0; i < 24; ++i )
      verts[i].color = color;
    size_t i = 0;
    verts[i++].pos = vec3( mi.x, mi.y, mi.z ); // mi,mi,mi
    verts[i++].pos = vec3( mi.x, mi.y, ma.z ); // to mi,mi,ma

    verts[i++].pos = vec3( mi.x, mi.y, ma.z ); // mi,mi,ma
    verts[i++].pos = vec3( ma.x, mi.y, ma.z ); // to ma,mi,ma

    verts[i++].pos = vec3( ma.x, mi.y, ma.z ); // ma,mi,ma
    verts[i++].pos = vec3( ma.x, mi.y, mi.z ); // to ma,mi,mi

    verts[i++].pos = vec3( ma.x, mi.y, mi.z ); // ma,mi,mi
    verts[i++].pos = vec3( mi.x, mi.y, mi.z ); // to mi,mi,mi

    verts[i++].pos = vec3( mi.x, mi.y, mi.z ); // mi,mi,mi
    verts[i++].pos = vec3( mi.x, ma.y, mi.z ); // to mi,ma,mi

    verts[i++].pos = vec3( mi.x, mi.y, ma.z ); // mi,mi,ma
    verts[i++].pos = vec3( mi.x, ma.y, ma.z ); // to mi,ma,ma

    verts[i++].pos = vec3( ma.x, mi.y, ma.z ); // ma,mi,ma
    verts[i++].pos = vec3( ma.x, ma.y, ma.z ); // to ma,ma,ma

    verts[i++].pos = vec3( ma.x, mi.y, mi.z ); // ma,mi,mi
    verts[i++].pos = vec3( ma.x, ma.y, mi.z ); // to ma,ma,mi

    verts[i++].pos = vec3( mi.x, ma.y, mi.z ); // mi,ma,mi
    verts[i++].pos = vec3( mi.x, ma.y, ma.z ); // to mi,ma,ma

    verts[i++].pos = vec3( mi.x, ma.y, ma.z ); // mi,ma,ma
    verts[i++].pos = vec3( ma.x, ma.y, ma.z ); // to ma,ma,ma

    verts[i++].pos = vec3( ma.x, ma.y, ma.z ); // ma,ma,ma
    verts[i++].pos = vec3( ma.x, ma.y, mi.z ); // to ma,ma,mi

    verts[i++].pos = vec3( ma.x, ma.y, mi.z ); // ma,ma,mi
    verts[i++].pos = vec3( mi.x, ma.y, mi.z ); // to mi,ma,mi
    boxviz_->buffer().unlock();
    auto ppl = &shaders.usePipeline( "dbg_line" );
    boxviz_->draw( *ppl, 24, 0, gl::GL_LINES );
  }

}