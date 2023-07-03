#include "pch.h"
#include "camera.h"
#include "locator.h"
#include "console.h"
#include "renderer.h"

namespace neko {

  Camera::Camera( vec2 resolution )
  {
    setViewport( resolution );
  }

  Camera::~Camera()
  {
  }

  void Camera::setViewport( vec2 resolution )
  {
    resolution_ = resolution;
  }

  const mat4 Camera::model() const noexcept
  {
    return glm::translate( mat4( 1.0f ), position_ );
  }

  void Camera::exposure( Real exp )
  {
    exposure_ = exp;
  }

}