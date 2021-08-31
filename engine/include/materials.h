#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "texture.h"

namespace neko {

  struct MaterialLayer: public nocopy {
  public:
    ImageData image_;
    TexturePtr texture_;
  public:
    inline const bool hasHostCopy() const { return !image_.data_.empty(); }
    inline const bool uploaded() const throw() { return ( texture_.get() != nullptr ); }
    MaterialLayer() {}
    // move constructor
    MaterialLayer( MaterialLayer&& rhs ): image_( move( rhs.image_ ) ), texture_( rhs.texture_ )
    {
    }
  };

  using MaterialLayers = vector<MaterialLayer>;

  struct Material: public nocopy {
    enum Type {
      UnlitSimple,
      WorldGround,
      WorldPBR,
      WorldUntexturedPBS
    } type_;
    bool loaded_;
    MaterialLayers layers_;
    explicit Material( Type type ): type_( type ), loaded_( false ) {}
    inline const bool uploaded() const throw()
    {
      if ( !loaded_ )
        return false;
      for ( const auto& layer : layers_ )
        if ( !layer.uploaded() )
          return false;
      return true;
    }
  };

  using MaterialPtr = shared_ptr<Material>;
  using MaterialVector = vector<MaterialPtr>;

  class MaterialManager {
  private:
  public:
  };

}