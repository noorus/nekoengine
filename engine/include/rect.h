#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "nekomath.h"

namespace neko {

  struct Rect
  {
    Real left_ = numbers::zero;
    Real top_ = numbers::zero;
    Real right_ = numbers::zero;
    Real bottom_ = numbers::zero;
    Rect( Real left, Real top, Real right, Real bottom ):
      left_( left ), top_( top ), right_( right ), bottom_( bottom ) {}
    inline void reset( Real left, Real top, Real right, Real bottom )
    {
      left_ = left;
      top_ = top;
      right_ = right;
      bottom_ = bottom;
    }
    inline vec2 minBound() const
    {
      return { left_, top_ };
    }
    inline vec2 maxBound() const
    {
      return { right_, bottom_ };
    }
  };

}
