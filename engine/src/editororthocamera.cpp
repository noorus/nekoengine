#include "pch.h"
#include "camera.h"
#include "locator.h"
#include "console.h"
#include "renderer.h"

namespace neko {

  EditorOrthoCamera::EditorOrthoCamera( vec2 resolution, const EditorViewportDefinition& def ):
    Camera( resolution, 90.0f )
  {
    position_ = def.position;
    eye_ = math::normalize( def.eye );
    up_ = def.up;
  }

  void EditorOrthoCamera::setViewport( vec2 resolution )
  {
    resolution_ = resolution;
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

  void EditorOrthoCamera::update( SManager& manager, GameTime delta, GameTime time )
  {
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

}