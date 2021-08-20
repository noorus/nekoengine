#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "texture.h"

namespace neko {

  struct MaterialLayer {
    ImageData image_;
    TexturePtr texture_;
    inline const bool hasHostCopy() const { return !image_.data_.empty(); }
    inline const bool uploaded() const throw() { return ( texture_.get() != nullptr ); }
  };

  using MaterialLayers = vector<MaterialLayer>;

  struct Material {
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