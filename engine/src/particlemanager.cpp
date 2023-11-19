#include "pch.h"
#include "gfx_types.h"
#include "renderer.h"
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
#include "particles.h"

namespace neko {

  ParticleSystemManager::ParticleSystemManager()
  {
  }

  void ParticleSystemManager::update( GameTime delta, GameTime time )
  {
  }

  void ParticleSystemManager::draw( Shaders& shaders, MaterialManager& materials )
  {
  }

  ParticleSystemManager::~ParticleSystemManager()
  {
  }

}