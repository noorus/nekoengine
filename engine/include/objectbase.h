#include "neko_types.h"
#include "nekomath.h"

namespace neko {

  class TransformableObject {
  protected:
    srt transform_;
  public:
    inline void setTransform( const vec3& scale, const quat& rotate, const vec3& translate )
    {
      transform_.scale = scale;
      transform_.rotate = rotate;
      transform_.translate = translate;
    }
    inline void setTranslation( const vec3& translate ) { transform_.translate = translate; }
    inline void setScale( const vec3& scale ) { transform_.scale = scale; }
    inline void setRotation( const quaternion& rotate ) { transform_.rotate = rotate; }
    inline const srt& transform() noexcept { return transform_; }
    inline void translate( const vec3& position ) { transform_.translate += position; }
    inline void scale( const vec3& scale ) { transform_.scale *= scale; }
    inline void rotate( const quaternion& rotation ) { transform_.rotate = math::normalize( rotation * transform_.rotate ); }
  };

}