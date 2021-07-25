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
    projection_ = glm::ortho( 0.0f, resolution.x, resolution.y, 0.0f, 0.1f, 1000.0f );
    projection_ = glm::perspective( glm::radians( 45.0f ), resolution.x / resolution.y, 0.1f, 100.0f );
  }

  void Camera::update( GameTime time )
  {
    position_ = vec3( math::sin( (Real)time * 0.5f ) * 6.0f, 6.0f, math::cos( (Real)time * 0.5f ) * 6.0f );
    view_ = glm::lookAt( position_, vec3( 0.0f, 0.0f, 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) );
    //view_ = glm::translate( position_ ); // glm::translate( vec3( -position_.x, -position_.y, position_.z ) );
  }

}