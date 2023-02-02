#pragma once
#include "forwards.h"
#include "neko_types.h"

namespace neko {

  class SceneManager;
  class SceneNode;

#undef near
#undef far

  class Camera {
  protected:
    vec2 resolution_;
    vec3 position_;
    mat4 view_;
    mat4 projection_;
    Real fov_;
    Real near_;
    Real far_;
    Real exposure_;
    SceneManager* manager_;
    SceneNode* node_;
  public:
    Camera( SceneManager* manager, vec2 viewport, Real fov );
    virtual ~Camera();
    virtual void setViewport( vec2 resolution );
    virtual void update( GameTime delta, GameTime time ) = 0;
    inline const vec2& resolution() const noexcept { return resolution_; }
    inline const mat4& view() const noexcept { return view_; }
    inline const mat4& projection() const noexcept { return projection_; }
    inline const vec3& position() const noexcept { return position_; }
    inline Real near() const noexcept { return near_; }
    inline Real far() const noexcept { return far_; }
    inline Real exposure() const noexcept { return exposure_; }
    void exposure( Real exp );
    virtual vec3 direction() const = 0;
    inline Real fovy() const { return fov_; }
    virtual Real aspect() const = 0;
    virtual vec3 right() const = 0;
    virtual vec3 up() const = 0;
  };

  struct EditorViewportDefinition
  {
    utf8String name;
    vec3 eye;
    vec3 up;
  };

  class EditorOrthoCamera: public Camera {
  protected:
    vec3 eye_;
    vec3 up_;
    Real orthoRadius_ = 14.0f;
    void _reposition();
  public:
    EditorOrthoCamera( SceneManager* manager, vec2 resolution, const EditorViewportDefinition& def );
    void setViewport( vec2 resolution ) override;
    void update( GameTime delta, GameTime time ) override;
    inline Real radius() noexcept { return orthoRadius_; }
    void radius( Real radius );
    vec3 direction() const override;
    virtual void applyInputPanning( const vec3i& mousemovement );
    virtual void applyInputZoom( int zoom );
  };

  struct CameraValueState
  {
    Real acceleration_;
    Real deceleration_;
    Real velocity_;
    CameraValueState( Real acceleration, Real deceleration ):
      acceleration_( acceleration ), deceleration_( deceleration ), velocity_( 0.0f )
    {
    }
  };

  class OrbitCamera: public Camera {
  protected:
    SceneNode* target_; //!< Target scenenode
    vec3 offset_; //!< Offset from target
    vec3 direction_; //!< Normalised direction vector
    vec3 rotationInput_; //!< Input accumulator for rotation
    vec3 movementInput_; //!< Input accumulator for movement
    quat rotation_; //!< Rotation velocity
    quat clampRotation_; //!< Rotation for changed clamp variables
    Real rotationProgress_; //!< Slerp Progress
    bool clamp_;
    CameraValueState zoom_;
    Real deceleration_; //!< Rotation deceleration constant
    Real sensitivity_; //!< Rotation sensitivity
    Real distanceMin_; //!< Minimum distance to target
    Real distanceMax_; //!< Maximum distance to target
    Radians clampTop_; //!< Minimum camera angle to Y axis
    Radians clampBottom_; //!< Maximum camera angle to Y axis
    bool reverseAxes_; // Reverse rotation axes?
  public:
    explicit OrbitCamera( SceneManager* manager, vec2 resolution, SceneNode* pgeTarget, const vec3& vecOffset, Real rFOVy,
      bool bReverseAxes, Real fSensitivity, Real fMinDistance, Real fMaxDistance, Real fRotationDeceleration,
      Real fZoomAcceleration, Real fZoomDeceleration );
    virtual void applyInputRotation( const vec3i& mousemovement );
    virtual void applyInputPanning( const vec3i& mousemovement );
    virtual void applyInputZoom( int zoom );
    void update( GameTime delta, GameTime time ) override;
    virtual const vec3& getDirection() const noexcept { return direction_; }
    virtual Real getSensitivity() { return sensitivity_; }
    virtual Real getMinDistance() { return distanceMin_; }
    virtual Real getMaxDistance() { return distanceMax_; }
    virtual Radians getClampTop() { return clampTop_; }
    virtual Radians getClampBottom() { return clampBottom_; }
    virtual void setSensitivity( Real fSensitivity );
    virtual void setMinDistance( Real fMinDistance );
    virtual void setMaxDistance( Real fMaxDistance );
    virtual void setClampTop( Radians fClampTop );
    virtual void setClampBottom( Radians fClampBottom );
    vec3 direction() const override;
  };

}