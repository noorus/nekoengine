#pragma once
#include "neko_types.h"

namespace neko {

  class aabb {
  private:
    vec3 min_;
    vec3 max_;
  public:
    aabb( const vec3& minimum, const vec3& maximum )
    {
      assert( minimum.x < maximum.x && minimum.y < maximum.y && minimum.z < maximum.y );
      min_ = minimum;
      max_ = maximum;
    }
    bool intersects( const vec3& p )
    {
      return ( p.x >= min_.x && p.x <= max_.x && p.y >= min_.y && p.y <= max_.y && p.z >= min_.z && p.z <= max_.z );
    }
    inline Real x() { return min_.x; }
    inline Real y() { return min_.y; }
    inline Real z() { return min_.z; }
    inline const vec3& min() { return min_; }
    inline const vec3& max() { return max_; }
    inline vec3 extents()
    {
      return ( max_ - min_ );
    }
    inline Real width()
    {
      return ( max_.x - min_.x );
    }
    inline Real height()
    {
      return ( max_.y - min_.y );
    }
    inline Real depth()
    {
      return ( max_.z - min_.z );
    }
  };

}