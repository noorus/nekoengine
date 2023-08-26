#include "pch.h"
#include "gfx_types.h"
#include "renderer.h"
#include "meshmanager.h"
#include "shaders.h"
#include "camera.h"
#include "loader.h"
#include "engine.h"
#include "font.h"
#include "lodepng.h"
#include "gfx.h"
#include "nekosimd.h"
#include "particles.h"
#include "math_aabb.h"
#include "filesystem.h"
#include "spriteanim.h"
#include "frustum.h"

namespace neko {

  using namespace gl;

  PaintableTexture::PaintableTexture( Renderer& renderer, int width, int height, PixelFormat format )
  {
    recreate( renderer, width, height, format );
  }

  PaintableTexture::PaintableTexture( Renderer& renderer, const Pixmap& pmp, PixelFormat format )
  {
    loadFrom( renderer, pmp, format );
  }

  void PaintableTexture::recreate( Renderer& renderer, int width, int height, PixelFormat format )
  {
    texture_.reset();
    vector<uint8_t> data;
    if ( format == PixFmtColorRGBA32f )
    {
      auto protoValue = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
      data.resize( width * height * 4 * sizeof( float ) );
      auto ptr = reinterpret_cast<vec4*>( data.data() );
      for ( auto i = 0; i < ( width * height ); ++i )
        ptr[i] = protoValue;
    }
    texture_ = renderer.createTexture(
      width, height, format,
      data.empty() ? nullptr : data.data(),
      Texture::Wrapping::Repeat,
      Texture::Filtering::Nearest,
      1 );
  }

  void PaintableTexture::loadFrom( Renderer& renderer, const Pixmap& pmp, PixelFormat format )
  {
    if ( !texture_
      || texture_->width() != pmp.width()
      || texture_->height() != pmp.height()
      || texture_->format() != format )
      recreate( renderer, pmp.width(), pmp.height(), format );

    if ( pmp.format() != format )
    {
      auto pmc = Pixmap::from( pmp );
      pmc.convert( format );
      texture_->writeData( pmc.data().data() );
    }
    else
      texture_->writeData( pmp.data().data() );
  }

  PixmapPtr PaintableTexture::read( Renderer& renderer )
  {
    if ( !texture_ )
      return {};

    return texture_->readBack();
  }

}