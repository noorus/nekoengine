#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "nekomath.h"

namespace neko {

  struct Plane
  {
    vec3 normal_ = vec3::unit_y();
    Real distance_ = 0.0f;
    Plane() = default;
    Plane( const Plane& rhs )
    {
      normal_ = rhs.normal_;
      distance_ = rhs.distance_;
    }
    Plane( Real a, Real b, Real c, Real d )
    {
      normal_ = { a, b, c };
      distance_ = d;
    }
    Plane( const vec3& pos, const vec3& norm ):
      normal_( math::normalize( norm ) ),
      distance_( math::dot( normal_, pos ) )
    {
    }
    inline void reset( Real a, Real b, Real c, Real d )
    {
      normal_ = { a, b, c };
      distance_ = d;
    }
    inline Real signedDistanceTo( const vec3& pos ) const
    {
      return math::dot( normal_, pos ) - distance_;
    }
    inline void normalize()
    {
      auto invl = numbers::one / math::length( normal_ );
      normal_ *= invl;
      distance_ *= invl;
    }
    inline vector<vec3> corners()
    {
      vector<vec3> out;
      auto v1 = vec3( 1.0f, 0.0f, 0.0f );
      auto v2 = math::cross( normal_, v1 );
      v1 = math::cross( v2, normal_ );
      auto center = -normal_ * distance_;
      out.emplace_back( center + v1 );
      out.emplace_back( center - v1 );
      out.emplace_back( center + v2 );
      out.emplace_back( center - v2 );
      return out;
    }
  };

}
