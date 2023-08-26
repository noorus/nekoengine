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

  // clang-format off

  template <size_t Count>
  class Particles {
  protected:
    static constexpr size_t c_particleCount = Count;
    neko_avx2_align vec4 positions_[c_particleCount];
    neko_avx2_align glm::f32quat orientations_[c_particleCount];
    neko_avx2_align vec4 velocities_[c_particleCount];
    neko_avx2_align vec4 acceleration_[c_particleCount];
    neko_avx2_align float masses_[c_particleCount];
    neko_avx2_align vec3 sizes_[c_particleCount];
    neko_avx2_align vec4 colors_[c_particleCount];
    bool gravity_ = false;
    unique_ptr<PointRenderBuffer<Count>> buffer_;
    GameTime time_ = 0.0;
    // position.w can be age if:
    // - velocity.w is kept 1
    // - acceleration.w is kept 1
  public:
    Particles()
    {
      buffer_ = make_unique<PointRenderBuffer<Count>>();
    }
    virtual void resetParticle( size_t index ) = 0;
    void resetSystem()
    {
      time_ = 0.0;
      for ( size_t i = 0; i < c_particleCount; ++i )
        resetParticle( i );
    }
    virtual void update( GameTime ddelta, GameTime time )
    {
      time_ += ddelta;
      float ftime = ( static_cast<float>( time_ ) * 6.0f );
      float mod_i = ( 1.2f + ( math::sin( 4.0f + ftime * numbers::pi * 0.1f ) * 0.8f ) ) * 0.5f;
      float mod_j = 1.0f - math::cos( ftime * numbers::pi );
      float mod_k = ( ( ( mod_i * 0.5f ) * ( mod_i * mod_i ) + ( mod_j * mod_i * 0.5f ) * 0.5f ) * 3.0f );
      float windStrength = 0.2f;
      auto wind = simd::vec8f(
        0.0f, mod_k * windStrength, 0.0f, 0.0f,
        0.0f, mod_k * windStrength, 0.0f, 0.0f );
      simd::vec8f delta( static_cast<float>( ddelta ) );
      const float s_gravity[8] = {
        0.0f, ( gravity_ ? -numbers::g : 0.0f ), 0.0f, 0.0f,
        0.0f, ( gravity_ ? -numbers::g : 0.0f ), 0.0f, 0.0f };
      simd::vec8f gravity( s_gravity );
      gravity *= delta;
      for ( size_t i = 0; i < ( c_particleCount / 8 ); ++i )
      {
        simd::vec8f mass_packed( &masses_[i * 8] );
        simd::vec8f mass[4];
        mass_packed.unpack8x4( mass[0], mass[1], mass[2], mass[3] );
        for ( size_t j = 0; j < 4; ++j )
        {
          size_t components_index = ( ( i * 8 ) + ( j * 2 ) );
          simd::vec8f acceleration( &acceleration_[components_index][0] );
          // f = m * ( g + w )
          auto forces = gravity + wind;
          simd::vec8f force = mass[j] * forces;
          // a = f / m
          acceleration = ( force / mass[j] );
          simd::vec8f velocity( &velocities_[components_index][0] );
          velocity = simd::vec8f::fma( acceleration, delta, velocity );
          simd::vec8f position( &positions_[components_index][0] );
          position = simd::vec8f::fma( velocity, delta, position );
          position.storeNontemporal( &positions_[components_index][0] );
          velocity.storeNontemporal( &velocities_[components_index][0] );
        }
      }
    }
    virtual void draw( Shaders& shaders, const Material& mat )
    {
      if ( !mat.uploaded() || mat.layers_.empty() || !mat.layers_[0].texture_ )
        return;
      auto points = buffer_->buffer().lock();
      for ( size_t i = 0; i < c_particleCount; ++i )
      {
        points[i].pos = positions_[i];
        points[i].orient = orientations_[i];
        points[i].size = sizes_[i];
        points[i].color = colors_[i];
      }
      buffer_->buffer().unlock();
      auto ppl = &shaders.usePipeline( "particle_world" );
      const auto hndl = mat.layers_[0].texture_->handle();
      gl::glBindTextures( 0, 1, &hndl );
      ppl->setUniform( "tex", 0 );
      buffer_->draw( *ppl, c_particleCount, 0 );
    }
  };

  using ParticleSystemPtr = shared_ptr<Particles<256>>;

  class SakuraSystem: public Particles<256> {
  protected:
    using Base = Particles<256>;
    using Base::c_particleCount;
    using Base::positions_;
    using Base::orientations_;
    using Base::velocities_;
    using Base::acceleration_;
    using Base::masses_;
    using Base::sizes_;
    using Base::colors_;
    neko_avx2_align vec3 axes_[c_particleCount];
    neko_avx2_align float rot_[c_particleCount];
    aabb box_;
    unique_ptr<LineRenderBuffer<24>> boxviz_;
  public:
    SakuraSystem( const aabb& box );
    virtual void resetParticle( size_t index );
    virtual void update( GameTime delta, GameTime time );
    virtual void draw( Shaders& shaders, const Material& mat );
  };

  class ParticleSystemManager {
  protected:
    unique_ptr<SakuraSystem> sakura_;
  public:
    ParticleSystemManager();
    void update( GameTime delta, GameTime time );
    void draw( Shaders& shaders, MaterialManager& materials );
    ~ParticleSystemManager();
  };

}