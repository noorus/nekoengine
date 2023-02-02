#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "nekomath.h"
#include "plane.h"
#include "frustum.h"
#include "transform.h"

namespace neko {

  struct BoundingVolume
  {
    virtual bool withinFrustum( const Frustum& frustum, const Transform& transform ) const = 0;
    virtual bool frontOfPlane( const Plane& plane ) const = 0;
    inline bool withinFrustum( const Frustum& frustum ) const
    {
      return ( frontOfPlane( frustum.left_ ) && frontOfPlane( frustum.right_ ) && frontOfPlane( frustum.top_ ) &&
               frontOfPlane( frustum.bottom_ ) && frontOfPlane( frustum.near_ ) && frontOfPlane( frustum.far_ ) );
    }
  };

  struct Sphere: public BoundingVolume
  {
    vec3 center_ = vec3( 0.0f );
    Real radius_ = 0.0f;
    Sphere( const vec3& center, Real radius ): center_( center ), radius_( radius ) {}
    bool frontOfPlane( const Plane& plane ) const override
    {
      return ( plane.signedDistanceTo( center_ ) > -radius_ );
    }
    bool withinFrustum( const Frustum& frustum, const Transform& transform ) const override
    {
      auto gscale = transform.globalScale();
      auto gcnt { transform.modelMatrix() * vec4( center_, 1.0f ) };
      auto maxScale = math::max( math::max( gscale.x, gscale.y ), gscale.z );
      Sphere gsphere( gcnt, radius_ * ( maxScale * 0.5f ) );
      return ( gsphere.frontOfPlane( frustum.left_ ) && gsphere.frontOfPlane( frustum.right_ ) &&
               gsphere.frontOfPlane( frustum.top_ ) && gsphere.frontOfPlane( frustum.bottom_ ) &&
               gsphere.frontOfPlane( frustum.near_ ) && gsphere.frontOfPlane( frustum.far_ ) );
    }
  };

  struct SquareAABB: public BoundingVolume
  {
    vec3 center_ = vec3( 0.0f );
    Real extent_ = 0.0f;
    SquareAABB( const vec3& center, Real inExtent ): center_( center ), extent_( inExtent ) {}
    bool frontOfPlane( const Plane& plane ) const override
    {
      const auto r = ( extent_ * ( math::abs( plane.normal_.x ) + math::abs( plane.normal_.y ) + math::abs( plane.normal_.z ) ) );
      return ( -r <= plane.signedDistanceTo( center_ ) );
    }
    bool withinFrustum( const Frustum& frustum, const Transform& transform ) const override
    {
      auto gcnt { transform.modelMatrix() * vec4( center_, 1.0f ) };
      auto right = transform.right() * extent_;
      auto up = transform.up() * extent_;
      auto forward = transform.forward() * extent_;

      const float newIi = math::abs( math::dot( vec3 { 1.f, 0.f, 0.f }, right ) ) +
                          math::abs( math::dot( vec3 { 1.f, 0.f, 0.f }, up ) ) +
                          math::abs( math::dot( vec3 { 1.f, 0.f, 0.f }, forward ) );

      const float newIj = math::abs( math::dot( vec3 { 0.f, 1.f, 0.f }, right ) ) +
                          math::abs( math::dot( vec3 { 0.f, 1.f, 0.f }, up ) ) +
                          math::abs( math::dot( vec3 { 0.f, 1.f, 0.f }, forward ) );

      const float newIk = math::abs( math::dot( vec3 { 0.f, 0.f, 1.f }, right ) ) +
                          math::abs( math::dot( vec3 { 0.f, 0.f, 1.f }, up ) ) +
                          math::abs( math::dot( vec3 { 0.f, 0.f, 1.f }, forward ) );

      const SquareAABB gb( gcnt, math::max( math::max( newIi, newIj ), newIk ) );

      return ( gb.frontOfPlane( frustum.left_ ) && gb.frontOfPlane( frustum.right_ ) &&
               gb.frontOfPlane( frustum.top_ ) && gb.frontOfPlane( frustum.bottom_ ) &&
               gb.frontOfPlane( frustum.near_ ) && gb.frontOfPlane( frustum.far_ ) );
    }
  };

  struct AABB: public BoundingVolume
  {
    vec3 center_ = vec3( 0.0f );
    vec3 extents_ = vec3( 0.0f );
    AABB( const vec3& min, const vec3& max ):
      center_ { ( max + min ) * 0.5f },
      extents_ { max.x - center_.x, max.y - center_.y, max.z - center_.z }
    {
    }
    AABB( const vec3& inCenter, Real iI, Real iJ, Real iK ): center_( inCenter ), extents_ { iI, iJ, iK } {}
    array<vec3, 8> vertices() const
    {
      array<vec3, 8> vertice {};
      vertice[0] = { center_.x - extents_.x, center_.y - extents_.y, center_.z - extents_.z };
      vertice[1] = { center_.x + extents_.x, center_.y - extents_.y, center_.z - extents_.z };
      vertice[2] = { center_.x - extents_.x, center_.y + extents_.y, center_.z - extents_.z };
      vertice[3] = { center_.x + extents_.x, center_.y + extents_.y, center_.z - extents_.z };
      vertice[4] = { center_.x - extents_.x, center_.y - extents_.y, center_.z + extents_.z };
      vertice[5] = { center_.x + extents_.x, center_.y - extents_.y, center_.z + extents_.z };
      vertice[6] = { center_.x - extents_.x, center_.y + extents_.y, center_.z + extents_.z };
      vertice[7] = { center_.x + extents_.x, center_.y + extents_.y, center_.z + extents_.z };
      return vertice;
    }
    bool frontOfPlane( const Plane& plane ) const override
    {
      const auto r = ( extents_.x * math::abs( plane.normal_.x ) + extents_.y * math::abs( plane.normal_.y ) +
                       extents_.z * math::abs( plane.normal_.z ) );
      return ( -r <= plane.signedDistanceTo( center_ ) );
    }
    bool withinFrustum( const Frustum& frustum, const Transform& transform ) const override
    {
      const auto gcnt { transform.modelMatrix() * vec4( center_, 1.0f ) };
      const auto right = transform.right() * extents_.x;
      const auto up = transform.up() * extents_.y;
      const auto forward = transform.forward() * extents_.z;

      const float newIi = math::abs( math::dot( vec3 { 1.f, 0.f, 0.f }, right ) ) +
                          math::abs( math::dot( vec3 { 1.f, 0.f, 0.f }, up ) ) +
                          math::abs( math::dot( vec3 { 1.f, 0.f, 0.f }, forward ) );

      const float newIj = math::abs( math::dot( vec3 { 0.f, 1.f, 0.f }, right ) ) +
                          math::abs( math::dot( vec3 { 0.f, 1.f, 0.f }, up ) ) +
                          math::abs( math::dot( vec3 { 0.f, 1.f, 0.f }, forward ) );

      const float newIk = math::abs( math::dot( vec3 { 0.f, 0.f, 1.f }, right ) ) +
                          math::abs( math::dot( vec3 { 0.f, 0.f, 1.f }, up ) ) +
                          math::abs( math::dot( vec3 { 0.f, 0.f, 1.f }, forward ) );

      const AABB gb( gcnt, newIi, newIj, newIk );

      return ( gb.frontOfPlane( frustum.left_ ) && gb.frontOfPlane( frustum.right_ ) && gb.frontOfPlane( frustum.top_ ) &&
               gb.frontOfPlane( frustum.bottom_ ) && gb.frontOfPlane( frustum.near_ ) && gb.frontOfPlane( frustum.far_ ) );
    }
  };

}
