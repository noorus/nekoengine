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
#include "particles.h"

namespace neko {

  ParticleSystemManager::ParticleSystemManager()
  {
  }

  void ParticleSystemManager::update( GameTime delta, GameTime time )
  {
    if ( !sakura_ )
      sakura_ = make_unique<SakuraSystem>( aabb( vec3( -10.0f, -2.0f, -10.0f ), vec3( 10.0f, 10.0f, 10.0f ) ) );
    sakura_->update( delta, time );
  }

  void ParticleSystemManager::draw( Shaders& shaders, MaterialManager& materials )
  {
    const auto partmat = materials.get( "demo_sakura" );
    sakura_->draw( shaders, *partmat );
  }

  ParticleSystemManager::~ParticleSystemManager()
  {
    //
  }

}