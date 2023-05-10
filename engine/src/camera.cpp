#include "pch.h"
#include "camera.h"
#include "locator.h"
#include "console.h"
#include "renderer.h"

namespace neko {

  EditorOrthoCamera::EditorOrthoCamera( SceneManager* manager, vec2 resolution, const EditorViewportDefinition& def ):
    Camera( manager, resolution, 90.0f )
  {
    position_ = def.position;
    eye_ = math::normalize( def.eye );
    up_ = def.up;
  }

  void EditorOrthoCamera::setViewport( vec2 resolution )
  {
    resolution_ = resolution;
    auto scale = 0.1f;
    _reposition();
  }

  void EditorOrthoCamera::_reposition()
  {
    aspect_ = resolution_.x / resolution_.y;
    auto diam = orthoRadius_;
    projection_ = glm::ortho(
      -( ( diam * 0.5f ) * aspect_ ), ( ( diam * 0.5f ) * aspect_ ), -( diam * 0.5f ), ( diam * 0.5f ), -1000.0f, 1000.0f );
  }

  void EditorOrthoCamera::applyInputPanning( const vec2& worldmov )
  {
    auto xmov = ( right() * ( worldmov.x ) );
    auto ymov = ( up() * ( worldmov.y ) );
    position_ += ( xmov + ymov );
    _reposition();
  }

  void EditorOrthoCamera::applyInputZoom( int zoom )
  {
    orthoRadius_ += -( (Real)zoom / (Real)WHEEL_DELTA );
    _reposition();
  }

  void EditorOrthoCamera::update( GameTime delta, GameTime time )
  {
    node_->setTranslate( position_ );
    view_ = glm::lookAt( position_, position_ + ( eye_ * 100.0f ), up() );
  }

  vec3 EditorOrthoCamera::direction() const
  {
    return eye_;
  }

  void EditorOrthoCamera::radius( Real radius )
  {
    orthoRadius_ = radius;
  }

  Real EditorOrthoCamera::aspect() const
  {
    return aspect_;
  }

  vec3 EditorOrthoCamera::right() const
  {
    return math::cross( direction(), up() );
  }

  vec3 EditorOrthoCamera::up() const
  {
    return up_;
  }

  Camera::Camera( SceneManager* manager, vec2 resolution, Degrees fov ):
  manager_( manager ), fov_( fov ), near_( 0.1f ), far_( 1000.0f ), exposure_( 1.0f )
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
    projection_ = glm::perspective( glm::radians( fov_ ), aspect(), near_, far_ );
  }

  Real Camera::aspect() const
  {
    return ( resolution_.x / resolution_.y );
  }

  const mat4 Camera::model() const noexcept
  {
    return glm::translate( mat4( 1.0f ), position_ );
  }

  void Camera::exposure( Real exp )
  {
    exposure_ = exp;
  }

  OrbitCamera::OrbitCamera( SceneManager* manager, vec2 resolution, SceneNode* target,
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

  void OrbitCamera::applyInputRotation( const vec3i& move )
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

  void OrbitCamera::applyInputPanning( const vec3i& move )
  {
    movementInput_ += vec3( 0.0f, -(Real)move.y, 0.0f );
  }

  void OrbitCamera::applyInputZoom( int zoom )
  {
    rotationInput_.z += -( (Real)zoom / (Real)WHEEL_DELTA );
  }

  void OrbitCamera::update( GameTime delta, GameTime time )
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

    //view_ = glm::lookAt( position_, target, vec3UnitY ); // glm::lookAt(glm::vec3(0.0f), CameraDirection, CameraUpVector);
    view_ = glm::lookAtRH( position_, target, up() );
  }

  vec3 OrbitCamera::direction() const
  {
    return math::normalize( target_->getDerivedTranslate() - position_ );
  }

  vec3 OrbitCamera::right() const
  {
    return math::cross( direction(), up() );
  }

  vec3 OrbitCamera::up() const
  {
    return { 0.0f, 1.0f, 0.0f };
  }

  void OrbitCamera::setSensitivity( Real sensitivity )
  {
    sensitivity_ = sensitivity;
  }

  void OrbitCamera::setMinDistance( Real dist )
  {
    distanceMin_ = dist;
    if ( math::length( offset_ ) < distanceMin_ )
    {
      offset_ = math::normalize( offset_ ) * distanceMin_;
      zoom_.velocity_ = 0.0f;
    }
  }

  void OrbitCamera::setMaxDistance( Real dist )
  {
    distanceMax_ = dist;
    if ( math::length( offset_ ) > distanceMax_ )
    {
      offset_ = math::normalize( offset_ ) * distanceMax_;
      zoom_.velocity_ = 0.0f;
    }
  }

  void OrbitCamera::setClampTop( Radians clamp )
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

  void OrbitCamera::setClampBottom( Radians clamp )
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