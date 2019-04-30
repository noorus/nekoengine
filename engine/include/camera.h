#pragma once
#include "forwards.h"
#include "neko_types.h"

namespace neko {

  class Camera {
  protected:
    vec2 resolution_;
    vec3 position_;
    mat4 view_;
    mat4 projection_;
  public:
    Camera( vec2 resolution, vec3 position );
    void update( GameTime delta, GameTime time );
    inline const mat4& view() const throw() { return view_; }
    inline const mat4& projection() const throw() { return projection_; }
  };

}