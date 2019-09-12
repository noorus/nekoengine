#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "textures.h"

namespace neko {

  struct Material {
    bool loaded_;
    ImageData image_;
    TexturePtr texture_;
    Material(): loaded_( false ) {}
  };

  using MaterialPtr = shared_ptr<Material>;

  using MaterialVector = vector<MaterialPtr>;

  class MaterialManager {
  private:
  public:
  };

}