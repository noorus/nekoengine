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
#include "gfx_imguistyle.h"

namespace neko {

  using namespace gl;

  void GameViewport::_update()
  {
    debugStr_ = utils::ilprinf( "game | %ix%i | %s", width_, height_, camdata_ ? camdata_->name.c_str() : "no camera" );
  }

  void GameViewport::setCameraData( const c::CameraData* data )
  {
    camdata_ = data;
    _update();
  }

  void GameViewport::resize( int width, int height, const Viewport& windowViewport )
  {
    windowResolution_ = windowViewport.sizef();
    Viewport::resize( width, height );
    _update();
  }

  bool GameViewport::drawopShouldDrawWireframe() const
  {
    return false;
  }

  vec2 GameViewport::drawopFullResolution() const
  {
    return windowResolution_;
  }

  vec3 GameViewport::drawopClearColor() const
  {
    return { 0.0f, 0.0f, 0.0f };
  }

  vec4 GameViewport::drawopGridColor() const
  {
    return { 0.0f, 0.0f, 0.0f, 0.0f };
  }

  Real GameViewport::drawopExposure() const
  {
    if ( camdata_ && camdata_->instance )
      return camdata_->instance->exposure();

    return 1.0f;
  }

  void GameViewport::drawopPreSceneDraw( shaders::Shaders& shaders ) const {}

  void GameViewport::drawopPostSceneDraw( shaders::Shaders& shaders ) const {}

  vec2 GameViewport::drawopViewportPosition() const
  {
    return position_;
  }

  vec2 GameViewport::drawopViewportSize() const
  {
    return { static_cast<Real>( width_ ), static_cast<Real>( height_ ) };
  }

  const CameraPtr GameViewport::drawopGetCamera() const
  {
    return ( camdata_ && camdata_->instance ? camdata_->instance : CameraPtr() );
  }

  bool GameViewport::drawopShouldDoBufferVisualizations() const
  {
    return true;
  }

  vec3 GameViewport::ndcPointToWorld( vec2 ndc_viewcoord ) const
  {
    return ndcPointToWorld( vec3( ndc_viewcoord, -1.0f ) );
  }

  vec3 GameViewport::ndcPointToWorld( vec3 ndc_viewcoord ) const
  {
    if ( !camdata_ || !camdata_->instance )
      return {};
    auto in = vec4( ndc_viewcoord, 1.0f );
    auto m = glm::inverse( camdata_->instance->frustum().projection() * camdata_->instance->view() );
    auto out = m * in;
    assert( out.w != 0.0f );
    out.w = 1.0f / out.w;
    return -vec3( out.z * out.w, out.y * out.w, out.x * out.w );
    //return { out.x * out.w, out.y * out.w, out.z * out.w };
  }

}