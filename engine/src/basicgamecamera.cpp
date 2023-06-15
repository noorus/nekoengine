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
    setViewport( viewport );
  }

  // clang-format off

  void BasicGameCamera::setViewport( vec2 resolution )
  {
    resolution_ = resolution;
    _reposition();
    projection_ = glm::perspective( glm::radians( fov_ ), aspect(), near_, far_ );
  }

  void BasicGameCamera::_reposition()
  {
    aspect_ = resolution_.x / resolution_.y;
  }

  void BasicGameCamera::update( SManager& manager, GameTime delta, GameTime time )
  {
    const auto& tfm = manager.tn( ent_ );
    position_ = tfm.derived_translate;
    direction_ = glm::rotate( tfm.derived_rotate, vec3( 0.0f, 0.0f, -1.0f ) );

    const auto& cam = manager.cam( ent_ );
    near_ = cam.nearDist;
    far_ = cam.farDist;
    exposure_ = cam.exposure;
    up_ = cam.up;

    if ( cam.projection == c::camera::CameraProjection::Perspective )
    {
      fov_ = cam.perspective_fovy;
      view_ = glm::lookAt( position_, { 0.0f, 0.0f, 0.0f }, up() );
      projection_ = glm::perspective( glm::radians( fov_ ), aspect(), near_, far_ );
    }
    else if ( cam.projection == c::camera::CameraProjection::Orthographic )
    {
      float aspect = aspect_;
      float radius = cam.orthographic_radius;
      view_ = glm::lookAt( position_, { 0.0f, 0.0f, 0.0f }, up() );
      projection_ = glm::ortho(
        -( ( radius * 0.5f ) * aspect ),
         ( ( radius * 0.5f ) * aspect ),
        -( radius * 0.5f ),
         ( radius * 0.5f ),
         near_, far_ );
    }
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