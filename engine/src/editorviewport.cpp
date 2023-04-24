#include "pch.h"
#include "neko_exception.h"
#include "engine.h"
#include "gfx.h"
#include "shaders.h"
#include "renderer.h"
#include "camera.h"
#include "font.h"
#include "console.h"
#include "messaging.h"
#include "gui.h"
#include "neko_types.h"

namespace neko {

  using namespace gl;

  EditorViewport::EditorViewport( SceneManager* manager, vec2 resolution, const EditorViewportDefinition& def ):
    name_( def.name )
  {
    axisMask_ = vec4( math::abs( def.eye.x ) > 0.1f ? 0.0f : 1.0f, math::abs( def.eye.y ) > 0.1f ? 0.0f : 1.0f,
      math::abs( def.eye.z ) > 0.1f ? 0.0f : 1.0f, 1.0f );
    camera_ = make_unique<EditorOrthoCamera>( manager, resolution, def );
  }

  void EditorViewport::resize( int width, int height, const Viewport& windowViewport )
  {
    windowResolution_ = windowViewport.sizef();
    Viewport::resize( width, height );
  }

  bool EditorViewport::drawopShouldDrawSky() const
  {
    return false;
  }

  bool EditorViewport::drawopShouldDrawWireframe() const
  {
    return false;
  }

  vec2 EditorViewport::drawopFullResolution() const
  {
    return windowResolution_;
  }

  vec3 EditorViewport::drawopClearColor() const
  {
    return { 0.0f, 0.0f, 0.0f };
  }

  Real EditorViewport::drawopExposure() const
  {
    return 1.0f;
  }

  vec3 EditorViewport::pointToWorld( vec2 point )
  {
    auto vmp = vec4( mapPointByViewport( point ), 0.0f, 1.0f );
    auto mat = glm::inverse( camera_->projection() * camera_->view() * camera_->model() );
    auto wc = ( camera_->model() * mat * vmp ) * axisMask_;
    return vec3 { -wc.z, -wc.y, -wc.x };
  }

  vec3 EditorViewport::windowPointToWorld( vec2 point )
  {
    auto vmp = vec4( mapPointByWindow( point ), 0.0f, 1.0f );
    auto mat = glm::inverse( camera_->projection() * camera_->view() * camera_->model() );
    auto wc = ( camera_->model() * mat * vmp ) * axisMask_;
    return vec3 { -wc.z, -wc.y, -wc.x };
  }

}