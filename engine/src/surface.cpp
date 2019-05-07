#include "stdafx.h"
#include "gfx_types.h"
#include "renderer.h"

namespace neko {

  Surface::Surface( Renderer* renderer, size_t width, size_t height, GLGraphicsFormat format ):
    width_( width ), height_( height ), format_( format ), handle_( 0 ),
    renderer_( renderer )
  {
    assert( renderer && width > 0 && height > 0 );
  }

}