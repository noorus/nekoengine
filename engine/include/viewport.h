#pragma once
#include "forwards.h"
#include "gfx_types.h"
#include "neko_types.h"
#include "neko_platform.h"

namespace neko {

  class Viewport {
  protected:
    vec2i position_ = { 0, 0 };
    int width_ = 0;
    int height_ = 0;
    mutable GLint previousSaved_[4] = { 0 };
  public:
    Viewport();
    Viewport( int width, int height );
    const bool valid();
    inline int width() const { return width_; }
    inline int height() const { return height_; }
    inline vec2i pos() const { return position_; }
    inline vec2i size() const { return { width_, height_ }; }
    inline vec2 posf() const { return position_; }
    inline vec2 sizef() const { return { static_cast<float>( width_ ), static_cast<float>( height_ ) }; }
    void move( int x, int y );
    void resize( int width, int height );
    inline void resize( vec2i size ) { resize( static_cast<int>( size.x ), static_cast<int>( size.y ) ); }
    void begin() const;
    void end() const;
    vec2 mapPointByViewport( vec2 point ) const; //!< Map a point from viewport-relative coordinates to view space
    vec2 mapPointByWindow( vec2 point ) const; //!< Map a point from window-relative coordinates to view space
  };

}