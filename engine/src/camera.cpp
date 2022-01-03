#include "pch.h"
#include "camera.h"
#include "locator.h"
#include "console.h"
#include "renderer.h"

namespace neko {

  Camera::Camera( SceneManager* manager, vec2 resolution, Degrees fov ):
  manager_( manager ), fov_( fov ), near_( 0.1f ), far_( 50.0f ), exposure_( 1.0f )
  {
    setViewport( resolution );
    node_ = manager_->createSceneNode();
  }

  Camera::~Camera()
  {
    manager_->destroySceneNode( node_ );
  }

  void Camera::setViewport( vec2 resolution )
  {
    resolution_ = resolution;
#if 1
    projection_ = glm::perspectiveFovRH( glm::radians( fov_ ), resolution.x, resolution.y, near_, far_ );
#else
    auto ratio = resolution_.x / resolution_.y;
    auto diam = 10.0f;
    projection_ = glm::orthoRH(
      -( ( diam * 0.5f ) * ratio ),
      ( ( diam * 0.5f ) * ratio ),
      -( diam * 0.5f ),
      ( diam * 0.5f ),
      0.01f, 1000.0f );
#endif
  }

  void Camera::exposure( Real exp )
  {
    exposure_ = exp;
  }

  ArcballCamera::ArcballCamera( SceneManager* manager, vec2 resolution, SceneNode* target,
    const vec3& offset, Degrees fov, bool reverse, Real sensitivity,
    Real fMinDistance, Real fMaxDistance, Real fRotationDeceleration, Real zoomAccel, Real zoomDecel )
      : Camera( manager, resolution, fov ), sensitivity_( sensitivity ),
        target_( target ), offset_( offset ), distanceMin_( fMinDistance ),
        distanceMax_( fMaxDistance ), deceleration_( fRotationDeceleration ),
        zoom_( zoomAccel, zoomDecel ),
        rotation_( quatIdentity ), rotationInput_( 0.0f ),
        clampTop_( 0.05f ), clampBottom_( glm::radians( 175.0f ) ),
        clampRotation_( quatIdentity ), clamp_( false ), direction_( 0.0f ), reverseAxes_( reverse ), rotationProgress_( 0.0f ), movementInput_( 0.0f )
  {
    assert( math::length( offset_ ) > distanceMin_ && math::length( offset_ ) < distanceMax_ );
    setMinDistance( distanceMin_ );
    setMaxDistance( distanceMax_ );
  }

  void ArcballCamera::applyInputRotation( const vec3i& move )
  {
    if ( reverseAxes_ )
    {
      rotationInput_.x += -( (Real)move.x / resolution_.x ) * sensitivity_;
      rotationInput_.y += ( (Real)move.y / resolution_.y ) * sensitivity_;
    }
    else
    {
      rotationInput_.x += -( (Real)move.x / resolution_.x ) * sensitivity_;
      rotationInput_.y += -( (Real)move.y / resolution_.y ) * sensitivity_;
    }
  }

  void ArcballCamera::applyInputPanning( const vec3i& move )
  {
    movementInput_ += vec3( 0.0f, -(Real)move.y, 0.0f );
  }

  void ArcballCamera::applyInputZoom( int zoom )
  {
    rotationInput_.z += -( (Real)zoom / (Real)WHEEL_DELTA );
  }

  void ArcballCamera::update( GameTime delta, GameTime time )
  {
    auto localX = math::normalize( vec3( -offset_.z, 0, offset_.x ) );

    auto mov_y =
      math::interpolateSmoothstep( 0.0f,
        movementInput_.y,
        math::min( math::abs( movementInput_.y ) * 0.01f, 1.0f )
      );
    target_->setTranslate( vec3( target_->translation().x,
      math::min( math::max( target_->translation().y + ( mov_y * (Real)delta ), -3.0f ), 6.0f ),
      target_->translation().z ) );
    movementInput_ *= 0.98f;

    if ( rotationInput_.x || rotationInput_.y )
    {
      Radians rot_y = glm::radians( rotationInput_.y );

      auto reset = false;
      auto angle_y = math::angleBetween( offset_, vec3UnitY );
      if ( rot_y > angle_y - clampTop_ )
      {
        rot_y = angle_y - clampTop_;
        reset = true;
      }
      else if ( rot_y < angle_y - clampBottom_ )
      {
        rot_y = angle_y - clampBottom_;
        reset = true;
      }
      auto rotX = glm::angleAxis( rot_y, localX );
      auto rotY = glm::angleAxis( glm::radians( rotationInput_.x ), vec3UnitY );
      if ( reset )
      {
        rotation_ = rotX * rotY;
      }
      else
      {
        rotation_ = rotation_ * rotX * rotY;
      }
    }

    if ( clamp_ )
    {
      rotationProgress_ += static_cast<Real>( delta );
      if ( rotationProgress_ > 1.0f )
      {
        clamp_ = false;
      }
      else
      {
        offset_ = glm::slerp( clampRotation_, quatIdentity, rotationProgress_ ) * offset_;
      }
    }

    if ( rotation_ != quatIdentity )
    {
      rotation_ = glm::slerp( rotation_, quatIdentity, deceleration_ * static_cast<Real>( delta ) );
      offset_ = rotation_ * offset_;
    }

    direction_ = math::normalize( offset_ );

    if ( rotationInput_.z )
      zoom_.velocity_ += rotationInput_.z;

    if ( zoom_.velocity_ )
    {
      offset_ += direction_ * zoom_.velocity_ * zoom_.acceleration_ * static_cast<Real>( delta );
      Real distance = math::length( offset_ );
      if ( distance > distanceMax_ )
      {
        offset_ = direction_ * distanceMax_;
        zoom_.velocity_ = 0.0f;
      }
      else if ( distance < distanceMin_ )
      {
        offset_ = direction_ * distanceMin_;
        zoom_.velocity_ = 0.0f;
      }
      else
      {
        if ( zoom_.velocity_ > 0.0f )
        {
          zoom_.velocity_ -= (Real)delta * zoom_.deceleration_;
          if ( zoom_.velocity_ < 0.0f )
            zoom_.velocity_ = 0.0f;
        }
        else
        {
          zoom_.velocity_ += (Real)delta * zoom_.deceleration_;
          if ( zoom_.velocity_ > 0.0f )
            zoom_.velocity_ = 0.0f;
        }
      }
    }

    rotationInput_ = vec3( 0.0f );

    auto target = target_->getDerivedTranslate();
    position_ = target + offset_;
    node_->setTranslate( position_ );

    view_ = glm::lookAt( position_, target, vec3UnitY );
  }

  void ArcballCamera::setSensitivity( Real sensitivity )
  {
    sensitivity_ = sensitivity;
  }

  void ArcballCamera::setMinDistance( Real dist )
  {
    distanceMin_ = dist;
    if ( math::length( offset_ ) < distanceMin_ )
    {
      offset_ = math::normalize( offset_ ) * distanceMin_;
      zoom_.velocity_ = 0.0f;
    }
  }

  void ArcballCamera::setMaxDistance( Real dist )
  {
    distanceMax_ = dist;
    if ( math::length( offset_ ) > distanceMax_ )
    {
      offset_ = math::normalize( offset_ ) * distanceMax_;
      zoom_.velocity_ = 0.0f;
    }
  }

  void ArcballCamera::setClampTop( Radians clamp )
  {
    clampTop_ = clamp;
    auto angle = math::angleBetween( offset_, vec3UnitY );
    if ( angle < clampTop_ )
    {
      auto axis = math::normalize( vec3( -offset_.z, 0, offset_.x ) );
      rotationProgress_ = 0.5f;
      clampRotation_ = glm::angleAxis( angle - clampTop_, axis );
      clamp_ = true;
    }
  }

  void ArcballCamera::setClampBottom( Radians clamp )
  {
    clampBottom_ = clamp;
    auto angle = math::angleBetween( offset_, vec3UnitY );
    if ( angle > clampBottom_ )
    {
      auto axis = math::normalize( vec3( -offset_.z, 0, offset_.x ) );
      rotationProgress_ = 0.5f;
      clampRotation_ = glm::angleAxis( angle - clampBottom_, axis );
      clamp_ = true;
    }
  }

}