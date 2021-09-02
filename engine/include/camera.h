#pragma once
#include "forwards.h"
#include "neko_types.h"

namespace neko {

  class SceneManager;
  class SceneNode;

  class Camera {
  protected:
    vec2 resolution_;
    vec3 position_;
    mat4 view_;
    mat4 projection_;
    Real fov_;
    SceneManager* manager_;
    SceneNode* node_;
  public:
    Camera( SceneManager* manager, vec2 viewport, Real fov );
    virtual ~Camera();
    void setViewport( vec2 resolution );
    virtual void update( GameTime delta, GameTime time ) = 0;
    inline const vec2& resolution() const throw() { return resolution_; }
    inline const mat4& view() const throw() { return view_; }
    inline const mat4& projection() const throw() { return projection_; }
    inline const vec3& position() const throw() { return position_; }
  };

  struct CameraValueState {
    Real acceleration_;
    Real deceleration_;
    Real velocity_;
    CameraValueState( Real acceleration, Real deceleration ): acceleration_( acceleration ), deceleration_( deceleration ), velocity_( 0.0f ) {}
  };

  class ArcballCamera: public Camera {
  protected:
    SceneNode* target_; //!< Target scenenode
    vec3 offset_; //!< Offset from target
    vec3 direction_; //!< Normalised direction vector
    vec3 rotationInput_; //!< Input accumulator for rotation
    vec3 movementInput_; //!< Input accumulator for movement
    quat rotation_; //!< Rotation velocity
    quat mClampRotation; //!< Rotation for changed clamp variables
    Real mClampRotProgress; //!< Slerp Progress
    bool clamp_;
    CameraValueState zoom_;
    Real mRotDeceleration; //!< Rotation deceleration constant
    Real mSensitivity; //!< Rotation sensitivity
    Real distanceMin_; //!< Minimum distance to target
    Real distanceMax_; //!< Maximum distance to target
    Radians clampTop_; //!< Minimum camera angle to Y axis
    Radians clampBottom_; //!< Maximum camera angle to Y axis
    bool reverseAxes_; // Reverse rotation axes?
  public:
    explicit ArcballCamera( SceneManager* manager, vec2 resolution, SceneNode* pgeTarget, const vec3& vecOffset, Real rFOVy, bool bReverseAxes, Real fSensitivity, Real fMinDistance, Real fMaxDistance, Real fRotationDeceleration, Real fZoomAcceleration, Real fZoomDeceleration );
    virtual void applyInputRotation( const vec3i& mousemovement );
    virtual void applyInputPanning( const vec3i& mousemovement );
    virtual void applyInputZoom( int zoom );
    virtual void update( GameTime delta, GameTime time );
    virtual const vec3& getDirection() const throw() { return direction_; }
    virtual Real getSensitivity() { return mSensitivity; }
    virtual Real getMinDistance() { return distanceMin_; }
    virtual Real getMaxDistance() { return distanceMax_; }
    virtual Radians getClampTop() { return clampTop_; }
    virtual Radians getClampBottom() { return clampBottom_; }
    virtual void setSensitivity( Real fSensitivity );
    virtual void setMinDistance( Real fMinDistance );
    virtual void setMaxDistance( Real fMaxDistance );
    virtual void setClampTop( Radians fClampTop );
    virtual void setClampBottom( Radians fClampBottom );
  };

}