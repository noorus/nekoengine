#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "nekomath.h"

namespace neko {

  struct Plane
  {
    vec3 normal_ = { 0.0f, 1.0f, 0.0f };
    Real distance_ = 0.0f;
    Plane() = default;
    Plane( const vec3& pos, const vec3& norm ): normal_( math::normalize( norm ) ), distance_( math::dot( normal_, pos ) ) {}
    inline Real signedDistanceTo( const vec3& pos ) const { return math::dot( normal_, pos ) - distance_; }
  };

}
