#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "nekomath.h"
#include "plane.h"
#include "camera.h"

namespace neko {

  struct Frustum
  {
    Plane top_;
    Plane bottom_;
    Plane right_;
    Plane left_;
    Plane far_;
    Plane near_;
    static Frustum createFrom( const Camera& camera )
    {
      Frustum f;
      auto halfV = camera.far() * math::tan( camera.fovy() * 0.5f );
      auto halfH = halfV * camera.aspect();
      auto multiplier = camera.far() * camera.direction();
      f.near_ = { camera.position() + camera.near() * camera.direction(), camera.direction() };
      f.far_ = { camera.position() + multiplier, -camera.direction() };
      f.right_ = { camera.position(), math::cross( multiplier - camera.right() * halfH, camera.up() ) };
      f.left_ = { camera.position(), math::cross( camera.up(), multiplier + camera.right() * halfH ) };
      f.top_ = { camera.position(), math::cross( camera.right(), multiplier - camera.up() * halfV ) };
      f.bottom_ = { camera.position(), math::cross( multiplier + camera.up() * halfV, camera.right() ) };
      return f;
    }
  };

}
