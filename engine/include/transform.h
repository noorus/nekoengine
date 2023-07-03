#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "nekomath.h"

namespace neko {

  class Transform {
  protected:
    vec3 translate_ = vec3( 0.0f );
    quaternion rotate_ = quat::identity();
    vec3 scale_ = vec3( 1.0f );
    mat4 mat_ = mat4( 1.0f ); //!< Cached model matrix
    bool dirty_ = true;
  protected:
    inline void needUpdate() { dirty_ = true; }
    inline mat4 makeModelMatrix()
    {
      auto rot = glm::toMat4( rotate_ );
      return math::translate( mat4( 1.0f ), translate_ ) * rot * math::scale( mat4( 1.0f ), scale_ );
    }
  public:
    inline void update()
    {
      mat_ = makeModelMatrix();
      dirty_ = false;
    }
    inline void update( const mat4& parentGlobalModel )
    {
      mat_ = ( parentGlobalModel * makeModelMatrix() );
      dirty_ = false;
    }
    inline void setTranslate( const vec3& position )
    {
      translate_ = position;
      needUpdate();
    }
    inline void setScale( const vec3& scale )
    {
      scale_ = scale;
      needUpdate();
    }
    inline void setRotate( const quaternion& rotate )
    {
      rotate_ = math::normalize( rotate );
      needUpdate();
    }
    inline void translate( const vec3& position )
    {
      translate_ += position;
      needUpdate();
    }
    inline void scale( const vec3& scale )
    {
      scale_ *= scale;
      needUpdate();
    }
    inline void rotate( const quaternion& rotation )
    {
      rotate_ = math::normalize( rotation * rotate_ );
      needUpdate();
    }
    inline const mat4& modelMatrix() const { return mat_; }
    inline const vec3& globalPosition() const { return mat_[3]; }
    inline vec3 right() const { return mat_[0]; }
    inline vec3 up() const { return mat_[1]; }
    inline vec3 backward() const { return mat_[2]; }
    inline vec3 forward() const { return -mat_[2]; }
    inline vec3 globalScale() const { return { math::length( right() ), math::length( up() ), math::length( backward() ) }; }
    inline bool dirty() const { return dirty_; }
  };

}
