#include "pch.h"
#include "gfx_types.h"
#include "renderer.h"
#include "camera.h"
#include "loader.h"
#include "engine.h"
#include "gfx.h"

namespace neko {

  using namespace gl;

  Viewport::Viewport()
  {
    //
  }

  Viewport::Viewport( int width, int height ):
    width_( width ), height_( height )
  {
    //
  }

  const bool Viewport::valid()
  {
    return ( position_.x >= 0 && position_.y >= 0 && width_ > 0 && height_ > 0 );
  }

  void Viewport::move( int x, int y )
  {
    position_.x = x;
    position_.y = y;
  }

  void Viewport::resize( int width, int height )
  {
    width_ = width;
    height_ = height;
  }

  vec2 Viewport::mapPointByViewport( vec2 point ) const
  {
    return { 2.0f * point.x / static_cast<float>( width_ ) - 1.0f, -( 2.0f * point.y / static_cast<float>( height_ ) - 1.0f ) };
  }

  vec2 Viewport::mapPointByWindow( vec2 point ) const
  {
    point -= vec2f( position_ );
    return mapPointByViewport( point );
  }

  void Viewport::begin() const
  {
    glGetIntegerv( GL_VIEWPORT, previousSaved_ );
    glViewportIndexedf( 0, static_cast<GLfloat>( position_.x ), static_cast<GLfloat>( position_.y ),
      static_cast<GLfloat>( width_ ), static_cast<GLfloat>( height_ ) );
  }

  void Viewport::end() const
  {
    glViewportIndexedf( 0, static_cast<GLfloat>( previousSaved_[0] ), static_cast<GLfloat>( previousSaved_[1] ),
      static_cast<GLfloat>( previousSaved_[2] ), static_cast<GLfloat>( previousSaved_[3] ) );
  }

}