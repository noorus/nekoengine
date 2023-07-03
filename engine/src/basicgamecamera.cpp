#include "pch.h"
#include "camera.h"
#include "locator.h"
#include "console.h"
#include "renderer.h"
#include "components.h"

namespace neko {

  BasicGameCamera::BasicGameCamera( vec2 viewport, SManager& manager, Entity e ):
  Camera( viewport ), ent_( e )
  {
    setViewport( viewport );
  }

  void BasicGameCamera::setViewport( vec2 resolution )
  {
    resolution_ = resolution;
  }

  void BasicGameCamera::update( SManager& manager, GameTime delta, GameTime time )
  {
    const auto& tfm = manager.tn( ent_ );
    position_ = tfm.translate;
    direction_ = glm::rotate( tfm.rotate, vec3( 0.0f, 0.0f, -1.0f ) );

    const auto& cam = manager.cam( ent_ );
    frustum_.near( cam.nearDist );
    frustum_.far( cam.farDist );
    frustum_.aspect( resolution_.x / resolution_.y );
    exposure_ = cam.exposure;
    up_ = cam.up;

    auto target = position_ + ( direction_ * cam.free_dist );
    if ( cam.tracking == c::camera::CameraTracking::Node )
      if ( manager.validAndTransform( cam.node_target ) )
      {
        target = manager.tn( cam.node_target ).translate;
      }
    
    view_ = glm::lookAt( position_, target, up() );

    if ( cam.projection == c::camera::CameraProjection::Perspective )
    {
      frustum_.type( Projection_Perspective );
      frustum_.fovy( math::radians( cam.perspective_fovy ) );
    }
    else if ( cam.projection == c::camera::CameraProjection::Orthographic )
    {
      frustum_.type( Projection_Orthographic );
      frustum_.radius( cam.orthographic_radius );
    }

    frustum_.update( view_, model() );
  }

  vec3 BasicGameCamera::direction() const
  {
    return direction_;
  }

  vec3 BasicGameCamera::right() const
  {
    return math::cross( direction(), up() );
  }

  vec3 BasicGameCamera::up() const
  {
    return up_;
  }
    
}