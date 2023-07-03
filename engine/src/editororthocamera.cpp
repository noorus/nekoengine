#include "pch.h"
#include "camera.h"
#include "locator.h"
#include "console.h"
#include "renderer.h"

namespace neko {

  EditorOrthoCamera::EditorOrthoCamera( vec2 resolution, const EditorViewportDefinition& def ):
    Camera( resolution )
  {
    position_ = def.position;
    eye_ = math::normalize( def.eye );
    up_ = def.up;
  }

  void EditorOrthoCamera::setViewport( vec2 resolution )
  {
    resolution_ = resolution;
  }

  void EditorOrthoCamera::applyInputPanning( const vec2& worldmov )
  {
    auto xmov = ( right() * ( worldmov.x ) );
    auto ymov = ( up() * ( worldmov.y ) );
    position_ += ( xmov + ymov );
  }

  void EditorOrthoCamera::applyInputZoom( int zoom )
  {
    orthoRadius_ += -( (Real)zoom / (Real)WHEEL_DELTA );
  }

  void EditorOrthoCamera::update( SManager& manager, GameTime delta, GameTime time )
  {
    view_ = glm::lookAt( position_, position_ + ( eye_ * 100.0f ), up() );

    frustum_.near( -1000.0f );
    frustum_.far( 1000.0f );
    frustum_.aspect( resolution_.x / resolution_.y );
    frustum_.type( Projection_Orthographic );
    frustum_.radius( orthoRadius_ );
    frustum_.update( view_, model() );
  }

  vec3 EditorOrthoCamera::direction() const
  {
    return eye_;
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