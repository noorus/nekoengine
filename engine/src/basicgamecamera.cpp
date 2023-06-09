#include "pch.h"
#include "camera.h"
#include "locator.h"
#include "console.h"
#include "renderer.h"
#include "components.h"

namespace neko {

  BasicGameCamera::BasicGameCamera( vec2 viewport, SManager& manager, Entity e ):
  Camera( viewport, 90.0f ), ent_( e )
  {
  }

  void BasicGameCamera::update( SManager& manager, GameTime delta, GameTime time )
  {
    const auto& tfm = manager.tn( ent_ );
    position_ = tfm.derived_translate;
    direction_ = glm::rotate( tfm.derived_rotate, vec3( 0.0f, 0.0f, -1.0f ) );

    const auto& cam = manager.cam( ent_ );
    fov_ = cam.fovy;
    near_ = cam.nearDist;
    far_ = cam.farDist;
    exposure_ = cam.exposure;
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
    return { 0.0f, 1.0f, 0.0f };
  }
    
}