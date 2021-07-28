#include "stdafx.h"
#include "camera.h"

namespace neko {

  Camera::Camera( vec2 resolution, vec3 position ): position_( position )
  {
    setViewport( resolution );
    view_ = glm::translate( position_ );
  }

  void Camera::setViewport( vec2 resolution )
  {
    resolution_ = resolution;
  }

  void Camera::update( GameTime time )
  {
    position_ = vec3( math::sin( (Real)time * 0.5f ) * 10.0f, math::sin( 1.5f + (Real)time ) * 4.0f + 3.0f, math::cos( (Real)time * 0.5f ) * 10.0f );
    projection_ = glm::infinitePerspective( glm::radians( 45.0f + ( math::sin( (Real)time ) * 12.0f ) ), resolution_.x / resolution_.y, 0.01f );
    view_ = glm::lookAt( position_, vec3( 0.0f, 0.0f, 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) );
  }

}