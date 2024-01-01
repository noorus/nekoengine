#pragma once
#include "forwards.h"
#include "neko_types.h"
#include "components.h"
#include "frustum.h"

namespace neko {

  class Camera {
  protected:
    vec2 resolution_;
    vec3 position_ { 0.0f };
    mat4 view_;
    Real exposure_ = 1.0f;
    Frustum frustum_;
  public:
    Camera( vec2 viewport );
    virtual ~Camera();
    virtual void setViewport( vec2 resolution );
    virtual void update( SManager& manager, GameTime delta, GameTime time ) = 0;
    inline const vec2& resolution() const noexcept { return resolution_; }
    inline const mat4& view() const noexcept { return view_; }
    inline const vec3& position() const noexcept { return position_; }
    inline const Frustum& frustum() const { return frustum_; }
    const mat4 model() const noexcept;
    inline Real exposure() const noexcept { return exposure_; }
    void exposure( Real exp );
    virtual vec3 direction() const = 0;
    virtual vec3 right() const = 0;
    virtual vec3 up() const = 0;
  };

  class GameCamera: public Camera {
  protected:
    Entity ent_ = c::null;
    vec3 direction_ { 0.0f, 0.0f, -1.0f };
    vec3 up_ = vec3::unit_y();
  public:
    GameCamera( vec2 viewport, SManager& manager, Entity e );
    void setViewport( vec2 resolution ) override;
    void update( SManager& manager, GameTime delta, GameTime time ) override;
    vec3 direction() const override;
    vec3 right() const override;
    vec3 up() const override;
  };

  struct EditorViewportDefinition
  {
    utf8String name;
    vec3 position;
    vec3 eye;
    vec3 up;
  };

  class EditorOrthoCamera: public Camera {
  protected:
    vec3 eye_;
    vec3 up_;
    Real orthoRadius_ = 14.0f;
  public:
    EditorOrthoCamera( vec2 resolution, const EditorViewportDefinition& def );
    void setViewport( vec2 resolution ) override;
    void update( SManager& manager, GameTime delta, GameTime time ) override;
    vec3 direction() const override;
    virtual void applyInputPanning( const vec2& worldmov );
    virtual void applyInputZoom( int zoom );
    vec3 right() const override;
    vec3 up() const override;
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

  /* class OrbitCamera: public Camera {
  protected:
    Entity target_;
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
    explicit OrbitCamera( vec2 resolution, Entity pgeTarget, const vec3& vecOffset, Real rFOVy,
      bool bReverseAxes, Real fSensitivity, Real fMinDistance, Real fMaxDistance, Real fRotationDeceleration,
      Real fZoomAcceleration, Real fZoomDeceleration );
    virtual void applyInputRotation( const vec3i& mousemovement );
    virtual void applyInputPanning( const vec3i& mousemovement );
    virtual void applyInputZoom( int zoom );
    void update( SManager& manager, GameTime delta, GameTime time ) override;
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
    vec3 right() const override;
    vec3 up() const override;
  };*/

}