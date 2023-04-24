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

  void GameViewport::setCamera( CameraPtr camera )
  {
    camera_ = camera;
  }

  void GameViewport::resize( int width, int height, const Viewport& windowViewport )
  {
    windowResolution_ = windowViewport.sizef();
    Viewport::resize( width, height );
  }

  bool GameViewport::drawopShouldDrawSky() const
  {
    return true;
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

  Real GameViewport::drawopExposure() const
  {
    if ( camera_ )
      return camera_->exposure();

    return 1.0f;
  }

}