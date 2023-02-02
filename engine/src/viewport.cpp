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

  void Viewport::resize( int width, int height )
  {
    width_ = width;
    height_ = height;
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