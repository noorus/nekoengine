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

  constexpr mat4 c_flipMat = mat4( 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f );

  vec2 ViewportDragOperation::remapNormal( vec2 vpcoord ) const
  {
    return ( vpcoord + 1.0f ) * 0.5f;
  }

  vec2 ViewportDragOperation::remapWorld( vec2 norm ) const
  {
    return ( norm * worlddims_ );
  }

  void ViewportDragOperation::begin( EditorViewportPtr vp, vec2 wincoord )
  {
    if ( vp_ != vp )
      end();

    vp_ = vp;
    startPos_ = remapNormal( vp_->mapPointByWindow( wincoord ) );
    dragging_ = true;
    worlddims_ = { vp_->camera()->radius() * vp_->camera()->aspect(), vp_->camera()->radius() };
  }

  void ViewportDragOperation::update( vec2 wincoord )
  {
    if ( !dragging_ || !vp_ )
      return;

    curPos_ = remapNormal( vp_->mapPointByWindow( wincoord ) );
  }

  vec2 ViewportDragOperation::getRelative() const
  {
    if ( !vp_ )
      return { 0.0f, 0.0f };

    return remapWorld( curPos_ - startPos_ );
  }

  void ViewportDragOperation::end()
  {
    dragging_ = false;
    vp_.reset();
  }

  EditorViewport::EditorViewport( EditorPtr editor, vec2 resolution, const EditorViewportDefinition& def ):
    name_( def.name ),
    editor_( editor )
  {
    axisMask_ = vec4(
      math::abs( def.eye.x ) > 0.1f ? 0.0f : 1.0f,
      math::abs( def.eye.y ) > 0.1f ? 0.0f : 1.0f,
      math::abs( def.eye.z ) > 0.1f ? 0.0f : 1.0f,
      1.0f );
    camera_ = make_unique<EditorOrthoCamera>( resolution, def );
    grid_ = make_unique<EditorGridRenderer>();
    axesGizmo_ = make_unique<AxesPointerRenderer>();
  }

  EditorViewport::~EditorViewport()
  {
    axesGizmo_.reset();
    grid_.reset();
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
    return editor_->clearColorRef();
  }

  vec4 EditorViewport::drawopGridColor() const
  {
    return editor_->gridColorRef();
  }

  Real EditorViewport::drawopExposure() const
  {
    return 1.0f;
  }

  void EditorViewport::drawopPreSceneDraw( shaders::Shaders& shaders ) const
  {
    grid_->update( *this, *camera_ );
    grid_->draw( shaders );
  }

  void EditorViewport::drawopPostSceneDraw( shaders::Shaders& shaders ) const
  {
    axesGizmo_->draw( shaders, viewportPointToWorld( { 40.0f, 70.0f, 0.0f } ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ) );
  }

  const CameraPtr EditorViewport::drawopGetCamera() const
  {
    return camera_;
  }

  vec3 EditorViewport::ndcPointToWorld( vec2 ndc_viewcoord ) const
  {
    return ndcPointToWorld( vec3( ndc_viewcoord, -1.0f ) );
  }

  vec3 EditorViewport::ndcPointToWorld( vec3 ndc_viewcoord ) const
  {
    auto in = vec4( ndc_viewcoord, 1.0f );
    auto m = glm::inverse( camera_->projection() * camera_->view() );
    auto out = m * in;
    assert( out.w != 0.0f );
    out.w = 1.0f / out.w;
    return ( c_flipMat * ( out * out.w * axisMask_ ) );
    // return vec3( out.x * out.w, out.y * out.w, out.z * out.w );
    //return -vec3( out.z * out.w, out.y * out.w, out.x * out.w );
  }

  /* vec3 EditorViewport::viewcoordPointToWorld( vec2 viewcoord ) const
  {
    auto vmp = vec4( viewcoord, 0.0f, 1.0f );
    auto mat = glm::inverse( camera_->projection() * camera_->view() * camera_->model() );
    auto wc = ( camera_->model() * mat * vmp ) * axisMask_;
    return { wc.x, wc.y, wc.z }; // { -wc.z, -wc.y, -wc.x };
  }*/

}