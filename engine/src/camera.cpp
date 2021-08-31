#include "stdafx.h"
#include "camera.h"
#include "locator.h"
#include "console.h"
#include "renderer.h"

namespace neko {

  Camera::Camera( SceneManager* manager, vec2 resolution, Degrees fov ): manager_( manager ), fov_( fov )
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
    projection_ = glm::infinitePerspective( glm::radians( fov_ ), resolution_.x / resolution_.y, 0.01f );
#else
    auto suhde = resolution_.x / resolution_.y;
    auto diam = 10.0f;
    projection_ = glm::orthoRH(
      -( ( diam * 0.5f ) * suhde ),
      ( ( diam * 0.5f ) * suhde ),
      -( diam * 0.5f ),
      ( diam * 0.5f ),
      0.01f, 1000.0f );
#endif
  }

  /*void Camera::update( GameTime time )
  {
    position_ = vec3( math::sin( (Real)time * 0.5f ) * 10.0f, 2.0f, math::cos( (Real)time * 0.5f ) * 10.0f );
    projection_ = glm::infinitePerspective( glm::radians( 60.0f ), resolution_.x / resolution_.y, 0.01f );
    view_ = glm::lookAt( position_, vec3( 0.0f, 0.0f, 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) );
  }*/

  ArcballCamera::ArcballCamera( SceneManager* manager, vec2 resolution, SceneNode* target,
    const vec3& offset, Degrees fov, bool reverse, Real sensitivity,
    Real fMinDistance, Real fMaxDistance, Real fRotationDeceleration, Real zoomAccel, Real zoomDecel )
      : Camera( manager, resolution, fov ), mSensitivity( sensitivity ),
        target_( target ), offset_( offset ), distanceMin_( fMinDistance ),
        distanceMax_( fMaxDistance ), mRotDeceleration( fRotationDeceleration ),
        zoom_( zoomAccel, zoomDecel ),
        rotation_( quatIdentity ), rotationInput_( 0.0f ),
        clampTop_( 0.05f ), clampBottom_( glm::radians( 175.0f ) ),
        mClampRotation( quatIdentity ), clamp_( false ), direction_( 0.0f ), reverseAxes_( reverse ), mClampRotProgress( 0.0f ), movementInput_( 0.0f )
  {
    assert( math::length( offset_ ) > distanceMin_ && math::length( offset_ ) < distanceMax_ );
    setMinDistance( distanceMin_ );
    setMaxDistance( distanceMax_ );
  }

  void ArcballCamera::applyInputRotation( const vec3i& move )
  {
    if ( reverseAxes_ )
    {
      rotationInput_.x += -( (Real)move.x / resolution_.x ) * mSensitivity;
      rotationInput_.y += ( (Real)move.y / resolution_.y ) * mSensitivity;
    }
    else
    {
      rotationInput_.x += -( (Real)move.x / resolution_.x ) * mSensitivity;
      rotationInput_.y += -( (Real)move.y / resolution_.y ) * mSensitivity;
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
      Radians rRotX = glm::radians( rotationInput_.x );
      Radians rRotY = glm::radians( rotationInput_.y );

      auto reset = false;
      auto angler = math::angleBetween( offset_, vec3UnitY );
      if ( rRotY > angler - clampTop_ )
      {
        rRotY = angler - clampTop_;
        reset = true;
      }
      else if ( rRotY < angler - clampBottom_ )
      {
        rRotY = angler - clampBottom_;
        reset = true;
      }
      auto rotX = glm::angleAxis( rRotY, localX );
      auto rotY = glm::angleAxis( rRotX, vec3UnitY );
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
      mClampRotProgress += 0.1f;
      if ( mClampRotProgress > 1.0f )
      {
        clamp_ = false;
      }
      else
      {
        auto qtClampRotation = glm::slerp( mClampRotation, quatIdentity, mClampRotProgress );
        offset_ = qtClampRotation * offset_;
      }
    }

    if ( rotation_ != quatIdentity )
    {
      rotation_ = glm::slerp( rotation_, quatIdentity, mRotDeceleration * (Real)delta );
      offset_ = rotation_ * offset_;
    }

    direction_ = math::normalize( offset_ );

    if ( rotationInput_.z )
      zoom_.velocity_ += rotationInput_.z;

    if ( zoom_.velocity_ )
    {
      offset_ += direction_ * zoom_.velocity_ * zoom_.acceleration_ * (Real)delta;
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
    mSensitivity = sensitivity;
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
      Radians rRotY = angle - clampTop_;
      auto vecAxis = math::normalize( vec3( -offset_.z, 0, offset_.x ) );
      mClampRotProgress = 0.5f;
      mClampRotation = glm::angleAxis( rRotY, vecAxis );
      clamp_ = true;
    }
  }

  void ArcballCamera::setClampBottom( Radians clamp )
  {
    clampBottom_ = clamp;
    auto angle = math::angleBetween( offset_, vec3UnitY );
    if ( angle > clampBottom_ )
    {
      Radians rRotY = angle - clampBottom_;
      auto vecAxis = math::normalize( vec3( -offset_.z, 0, offset_.x ) );
      mClampRotProgress = 0.5f;
      mClampRotation = glm::angleAxis( rRotY, vecAxis );
      clamp_ = true;
    }
  }

}