#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "nekomath.h"
#include "plane.h"
#include "camera.h"
#include "rect.h"

namespace neko {

# undef near
# undef far

  enum ProjectionType
  {
    Projection_Perspective = 0,
    Projection_Orthographic
  };

  enum FrustumPlaneIndex
  {
    FrustumPlane_Near = 0,
    FrustumPlane_Far,
    FrustumPlane_Left,
    FrustumPlane_Right,
    FrustumPlane_Top,
    FrustumPlane_Bottom,
    MAX_FrustumPlane
  };

  struct Frustum {
  private:
    ProjectionType projectionType_ = Projection_Perspective; // projection type
    Real near_ = 0.1f; // near clip distance
    Real far_ = 100.0f; // far clip distance
    Real aspect_ = 1.3333f; // viewport aspect ratio
    Radians fovy_ { numbers::pi / 4.0f }; // fov height in radians
    Real orthoHeight_ = 14.0f; // orthographic radius
    mutable mat4 view_ = glm::identity<mat4>();
    mutable mat4 model_ = glm::identity<mat4>();
    mutable Rect extents_ { 0.0f, 0.0f, 0.0f, 0.0f };
    mutable mat4 projection_ = glm::identity<mat4>();
    mutable array<vec3, 8> corners_ {}; // world-space corner points of the frustum
    optional<mat4> customProjection_;
    mutable array<Plane, MAX_FrustumPlane> planes_ {}; // generated frustum planes
    // these are untouched/unused currently
    Real focal_ = 1.0f;
    vec2 offset_ { numbers::zero, numbers::zero };
  protected:
    inline void _updateParameters() const
    {
      if ( customProjection_ )
      {
        auto ip = math::inverse( customProjection_.value() );
        auto topleft = vec4( -numbers::one, numbers::one, -numbers::one, numbers::one );
        auto botright = vec4( numbers::one, -numbers::one, -numbers::one, numbers::one );
        topleft = ip * topleft;
        botright = ip * botright;
        topleft /= topleft.w;
        botright /= botright.w;
        extents_.reset( topleft.x, topleft.y, botright.x, botright.y );
      }
      else if ( projectionType_ == Projection_Perspective )
      {
        auto thy = math::tan( fovy_ * numbers::half );
        auto thx = thy * aspect_;
        auto nearoff = offset_ * ( near_ / focal_ );
        auto halves = vec2( thx, thy ) * near_;
        extents_.reset( -halves.x + nearoff.x, +halves.y + nearoff.y, +halves.x + nearoff.x, -halves.y + nearoff.y );
      }
      else if ( projectionType_ == Projection_Orthographic )
      {
        auto halves = orthoWindow() * numbers::half;
        extents_.reset( -halves.x, +halves.y, +halves.x, -halves.y );
      }
      else
        NEKO_EXCEPT( "Unknown frustum projection type" );
    }
    inline void _updateFrustum() const
    {
      if ( !customProjection_ )
      {
        if ( projectionType_ == Projection_Perspective )
        {
          projection_ = glm::perspectiveFovRH_NO( fovy_, extents_.width(), extents_.height(), near_, far_ );
          /* projection_ = math::buildPerspective( extents_.left_, extents_.right_, extents_.bottom_, extents_.top_, near_, far_ );*/
        }
        else if ( projectionType_ == Projection_Orthographic )
        {
          projection_ =
            math::buildOrthographic( extents_.left_, extents_.right_, extents_.bottom_, extents_.top_, near_, far_ );
        }
      }
      else
        projection_ = customProjection_.value();
    }
    inline void _updatePlanes() const
    {
      auto pv = projection_ * view_;

      auto& left = planes_[FrustumPlane_Left];
      left.normal_.x = pv[3][0] + pv[0][0];
      left.normal_.y = pv[3][1] + pv[0][1];
      left.normal_.z = pv[3][2] + pv[0][2];
      left.distance_ = pv[3][3] + pv[0][3];

      auto& right = planes_[FrustumPlane_Right];
      right.normal_.x = pv[3][0] - pv[0][0];
      right.normal_.y = pv[3][1] - pv[0][1];
      right.normal_.z = pv[3][2] - pv[0][2];
      right.distance_ = pv[3][3] - pv[0][3];

      auto& top = planes_[FrustumPlane_Top];
      top.normal_.x = pv[3][0] - pv[1][0];
      top.normal_.y = pv[3][1] - pv[1][1];
      top.normal_.z = pv[3][2] - pv[1][2];
      top.distance_ = pv[3][3] - pv[1][3];

      auto& bottom = planes_[FrustumPlane_Bottom];
      bottom.normal_.x = pv[3][0] + pv[1][0];
      bottom.normal_.y = pv[3][1] + pv[1][1];
      bottom.normal_.z = pv[3][2] + pv[1][2];
      bottom.distance_ = pv[3][3] + pv[1][3];

      auto& nearp = planes_[FrustumPlane_Near];
      nearp.normal_.x = pv[3][0] + pv[2][0];
      nearp.normal_.y = pv[3][1] + pv[2][1];
      nearp.normal_.z = pv[3][2] + pv[2][2];
      nearp.distance_ = pv[3][3] + pv[2][3];

      auto& farp = planes_[FrustumPlane_Far];
      farp.normal_.x = pv[3][0] - pv[2][0];
      farp.normal_.y = pv[3][1] - pv[2][1];
      farp.normal_.z = pv[3][2] - pv[2][2];
      farp.distance_ = pv[3][3] - pv[2][3];

      for ( auto& p : planes_ )
        p.normalize();
    }
    inline void _updateCorners() const
    {
      auto eye = mat34( math::inverse( view_ ) );
      auto nr = extents_;
      auto ratio = ( projectionType_ == Projection_Perspective ? far_ / near_ : numbers::one );
      Rect fr( nr.left_ * ratio, nr.top_ * ratio, nr.right_ * ratio, nr.bottom_ * ratio );
      // near
      corners_[0] = ( eye * vec3( nr.right_, nr.top_, -near_ ) ) /* model_*/;
      corners_[1] = ( eye * vec3( nr.left_, nr.top_, -near_ ) ) /* model_*/;
      corners_[2] = ( eye * vec3( nr.left_, nr.bottom_, -near_ ) ) /* model_*/;
      corners_[3] = ( eye * vec3( nr.right_, nr.bottom_, -near_ ) ) /* model_*/;
      // far
      corners_[4] = ( eye * vec3( fr.right_, fr.top_, -far_ ) ) /* model_*/;
      corners_[5] = ( eye * vec3( fr.left_, fr.top_, -far_ ) ) /* model_*/;
      corners_[6] = ( eye * vec3( fr.left_, fr.bottom_, -far_ ) ) /* model_*/;
      corners_[7] = ( eye * vec3( fr.right_, fr.bottom_, -far_ ) ) /* model_*/;
    }
  public:
    Frustum() = default;
    inline const array<Plane, MAX_FrustumPlane>& planes() const { return planes_; }
    inline const Plane& nearPlane() const { return planes_[FrustumPlane_Near]; }
    inline const Plane& farPlane() const { return planes_[FrustumPlane_Far]; }
    inline const Plane& leftPlane() const { return planes_[FrustumPlane_Left]; }
    inline const Plane& rightPlane() const { return planes_[FrustumPlane_Right]; }
    inline const Plane& topPlane() const { return planes_[FrustumPlane_Top]; }
    inline const Plane& bottomPlane() const { return planes_[FrustumPlane_Bottom]; }
    inline const vec2& offset() const { return offset_; }
    inline const Rect& extents() const { return extents_; }
    inline ProjectionType type() const noexcept { return projectionType_; }
    inline void type( ProjectionType newType ) { projectionType_ = newType; }
    inline Radians fovy() const noexcept { return fovy_; }
    inline void fovy( Radians newFovY ) { fovy_ = newFovY; }
    inline Real near() const noexcept { return near_; }
    inline void near( Real newNear ) { near_ = newNear; }
    inline Real far() const noexcept { return far_; }
    inline void far( Real newFar ) { far_ = newFar; }
    inline Real aspect() const noexcept { return aspect_; }
    inline void aspect( Real newAspect ) { aspect_ = newAspect; }
    inline Real radius() const noexcept { return orthoHeight_; }
    inline void radius( Real newRadius ) { orthoHeight_ = newRadius; }
    inline Real orthoHeight() const noexcept { return orthoHeight_; }
    inline Real orthoWidth() const noexcept { return ( orthoHeight_ * aspect_ ); }
    inline vec2 orthoWindow() const { return { orthoHeight_ * aspect_, orthoHeight_ }; }
    inline const mat4& projection() const { return projection_; }
    inline const mat4& view() const { return view_; }
    inline const mat4& model() const { return model_; }
    inline void update( const mat4& vview, const mat4& vmodel )
    {
      view_ = vview;
      model_ = vmodel;
      _updateParameters();
      _updateFrustum();
      _updatePlanes();
      _updateCorners();
    }
    inline void feedVertsTo( span<VertexLine>& verts, size_t& i ) const
    {
      assert( i + 24 <= verts.size() );

      // near
      verts[i++].pos = corners_[0];
      verts[i++].pos = corners_[1];
      verts[i++].pos = corners_[1];
      verts[i++].pos = corners_[2];
      verts[i++].pos = corners_[2];
      verts[i++].pos = corners_[3];
      verts[i++].pos = corners_[3];
      verts[i++].pos = corners_[0];

      // far
      verts[i++].pos = corners_[4];
      verts[i++].pos = corners_[5];
      verts[i++].pos = corners_[5];
      verts[i++].pos = corners_[6];
      verts[i++].pos = corners_[6];
      verts[i++].pos = corners_[7];
      verts[i++].pos = corners_[7];
      verts[i++].pos = corners_[4];

      // connections
      verts[i++].pos = corners_[0];
      verts[i++].pos = corners_[4];
      verts[i++].pos = corners_[1];
      verts[i++].pos = corners_[5];
      verts[i++].pos = corners_[2];
      verts[i++].pos = corners_[6];
      verts[i++].pos = corners_[3];
      verts[i++].pos = corners_[7];
    }
  };

}
