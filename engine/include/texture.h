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
    Texture( Renderer* renderer, size_t width, size_t height, PixelFormat format, const void* data,
      const Wrapping wrapping, const Filtering filtering, int multisamples = 1 );
    ~Texture();
  };

  using TexturePtr = shared_ptr<Texture>;

}