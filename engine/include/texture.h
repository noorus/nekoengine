#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "surface.h"

namespace neko {

  class Renderer;

  //! \class Texture
  //! \brief A texture is the fundamental resource encapsulating one (or sometimes more) images
  //!   to be sampled while drawing primitives. Textures (as single GPU units) can vary
  //!   from a 1D pixel strip to an array of cubenamps, and can be created with any graphics format.
  //!   Textures are the most flexible surface/buffer primitive, being able to be uploaded, downloaded,
  //!   sampled, and modified.
  class Texture: public Surface {
  public:
    enum Type {
      Tex1D,
      Tex2D,
      Tex3D,
      Tex1DArray,
      Tex2DArray,
      Tex3DArray,
      TexCubemap,
      TexCubemapArray,
      Tex2DMultisample,
      Tex2DMultisampleArray
    };
    enum Wrapping {
      ClampEdge,
      ClampBorder,
      MirroredRepeat,
      Repeat,
      MirroredClampEdge
    };
    enum Filtering {
      Nearest,
      Linear,
      Mipmapped
    };
  protected:
    Type type_; //!< Texture type.
    int multisamples_;
  public:
    Texture() = delete;
    Texture( Renderer* renderer, int width, int height, PixelFormat format, const void* data,
      const Wrapping wrapping, const Filtering filtering, int multisamples = 1 );
    Texture( Renderer* renderer, int width, int height, int depth, PixelFormat format,
      const void* data, const Wrapping wrapping, const Filtering filtering, int multisamples = 1 );
    PixmapPtr readBack();
    void writeData( const void* data );
    void writeRect( const vec2i& offset, int width, int height, const void* data );
    ~Texture();
  };

  using TexturePtr = shared_ptr<Texture>;

  class PaintableTexture {
  protected:
    TexturePtr texture_;
  public:
    PaintableTexture( Renderer& renderer, int width, int height, PixelFormat format );
    PaintableTexture( Renderer& renderer, const Pixmap& pmp, PixelFormat format );
    void recreate( Renderer& renderer, int width, int height, PixelFormat format );
    void loadFrom( Renderer& renderer, const Pixmap& pmp, PixelFormat format );
    PixmapPtr read( Renderer& renderer );
    inline GLuint handle() const noexcept { return ( texture_ ? texture_->handle() : 0 ); }
    inline const TexturePtr texture() const noexcept { return texture_; }
    inline int width() const noexcept { return ( texture_ ? texture_->width() : 0 ); }
    inline int height() const noexcept { return ( texture_ ? texture_->height() : 0 ); }
  };

  using PaintableTexturePtr = shared_ptr<PaintableTexture>;

}