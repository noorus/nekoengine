#include "stdafx.h"
#include "camera.h"

namespace neko {

  const Real cWindowSize = 720.0f;

  Camera::Camera( vec2 resolution, vec3 position ): position_( position )
  {
    setViewport( resolution );
    view_ = glm::translate( position_ );
  }

  void Camera::setViewport( vec2 resolution )
  {
    projection_ = glm::ortho( 0.0f, resolution.x, 0.0f, resolution.y );
  }

  void Camera::update( GameTime delta, GameTime time )
  {
    view_ = glm::translate( position_ ); // glm::translate( vec3( -position_.x, -position_.y, position_.z ) );
  }

}