#pragma once
#include "forwards.h"
#include "gfx_types.h"
#include "neko_types.h"
#include "neko_platform.h"

namespace neko {

  // clang-format off

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
    inline float aspect() const { return static_cast<float>( width_ ) / static_cast<float>( height_ ); }
    void move( int x, int y );
    void resize( int width, int height );
    inline void resize( vec2i size ) { resize( static_cast<int>( size.x ), static_cast<int>( size.y ) ); }
    void begin() const;
    void end() const;
    virtual vec2 mapPointByViewport( vec2 point ) const; //!< Map a point from viewport-relative pixel coordinates to NDC view space
    virtual vec2 mapPointByWindow( vec2 point ) const; //!< Map a point from window-relative pixel coordinates to NDC view space
    virtual vec3 ndcPointToWorld( vec2 ndc_viewcoord ) const; //!< Cast a point on the 2D viewport in -1...1 view coordinates to world (near plane depth)
    virtual vec3 ndcPointToWorld( vec3 ndc_viewcoord ) const; //!< Cast a point on the 2D viewport in -1...1 view coordinates to world, Z maps to clip planes
    virtual bool ndcRay( vec2 ndc_viewcoord, Ray& ray ) const;
    virtual vec3 viewportPointToWorld( vec2 px_viewcoord ) const; //!< Cast a point from viewport-relative pixel coordinates to world (near plane depth)
    virtual vec3 viewportPointToWorld( vec3 px_viewcoord ) const; //!< Cast a point from viewport-relative pixel coordinates to world, Z maps to clip planes
    virtual bool viewportRay( vec2 px_viewcoord, Ray& ray ) const;
    virtual vec3 windowPointToWorld( vec2 px_windowcoord ) const; //!< Cast a point from window-relative pixel coordinates to world (near plane depth)
    virtual vec3 windowPointToWorld( vec3 px_windowcoord ) const; //!< Cast a point from window-relative pixel coordinates to world, Z maps to clip planes
    virtual bool windowRay( vec2 px_viewcoord, Ray& ray ) const;
  };

}