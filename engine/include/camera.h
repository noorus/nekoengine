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
    Camera( vec2 viewport, vec3 position );
    void setViewport( vec2 resolution );
    void update( GameTime time );
    inline const vec2& resolution() const throw() { return resolution_; }
    inline const mat4& view() const throw() { return view_; }
    inline const mat4& projection() const throw() { return projection_; }
    inline const vec3& position() const throw() { return position_; }
  };

}