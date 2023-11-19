#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "neko_platform.h"
#include "texture.h"
#include "framebuffer.h"
#include "renderbuffer.h"
#include "materials.h"
#include "meshmanager.h"
#include "modelmanager.h"
#include "scripting.h"
#include "shaders.h"
#include "math_aabb.h"
#include "buffers.h"

namespace neko {

  class ParticleSystemManager {
  public:
    ParticleSystemManager();
    void update( GameTime delta, GameTime time );
    void draw( Shaders& shaders, MaterialManager& materials );
    ~ParticleSystemManager();
  };

}