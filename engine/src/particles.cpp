#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"
#include "meshmanager.h"
#include "shaders.h"
#include "camera.h"
#include "loader.h"
#include "engine.h"
#include "fontmanager.h"
#include "lodepng.h"
#include "gfx.h"
#include "nekosimd.h"
#include "particles.h"

namespace neko {

  using namespace gl;

  SakuraSystem::SakuraSystem( const aabb& box ): Base(), box_( box )
  {
    resetSystem();
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

}