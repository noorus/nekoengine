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

namespace neko {

  #pragma pack( push, 1 )
  struct VertexPointRender
  {
    vec3 pos;
    glm::f32quat orient;
    vec3 size;
    vec4 color;
  };
#pragma pack( pop )

  class PointRenderBuffer
  {
  protected:
    const GLuint c_maxVertices = 256;
    unique_ptr<SmarterBuffer<VertexPointRender>> buffer_;
    GLuint vao_;

  public:
    PointRenderBuffer()
    {
      buffer_ = make_unique<SmarterBuffer<VertexPointRender>>( c_maxVertices );
      gl::glCreateVertexArrays( 1, &vao_ );
      neko::AttribWriter attribs;
      attribs.add( gl::GL_FLOAT, 3 ); // vec3 position
      attribs.add( gl::GL_FLOAT, 4 ); // quat orientation
      attribs.add( gl::GL_FLOAT, 3 ); // vec3 size
      attribs.add( gl::GL_FLOAT, 4 ); // vec4 color
      attribs.write( vao_ );
      gl::glVertexArrayVertexBuffer( vao_, 0, buffer_->id(), 0, attribs.stride() );
    }
    inline SmarterBuffer<VertexPointRender>& buffer() { return *buffer_.get(); }
    void draw( shaders::Pipeline& pipeline, GLsizei count, GLint base = 0 )
    {
      gl::glBindVertexArray( vao_ );
      mat4 mdl( 1.0f );
      pipeline.setUniform( "model", mdl );
      gl::glDrawArrays( gl::GL_POINTS, base, count );
      gl::glBindVertexArray( 0 );
    }
    ~PointRenderBuffer()
    {
      gl::glDeleteVertexArrays( 1, &vao_ );
      buffer_.reset();
    }
  };

  template <size_t Count>
  class Particles
  {
  protected:
    static constexpr size_t c_particleCount = Count;
    neko_avx_align vec4 positions_[c_particleCount];
    neko_avx_align glm::f32quat orientations_[c_particleCount];
    neko_avx_align vec4 velocities_[c_particleCount];
    neko_avx_align vec4 acceleration_[c_particleCount];
    neko_avx_align float masses_[c_particleCount];
    neko_avx_align vec3 sizes_[c_particleCount];
    neko_avx_align vec4 colors_[c_particleCount];
    bool gravity_ = false;
    unique_ptr<PointRenderBuffer> buffer_;
    GameTime time_;
    // position.w can be age if:
    // - velocity.w is kept 1
    // - acceleration.w is kept 1
  public:
    Particles()
    {
      buffer_ = make_unique<PointRenderBuffer>();
    }
    virtual void resetParticle( size_t index ) = 0;
    void resetSystem()
    {
      time_ = 0.0;
      for ( size_t i = 0; i < c_particleCount; ++i )
        resetParticle( i );
    }
    virtual void update( GameTime ddelta )
    {
      time_ += ddelta;
      float ftime = ( static_cast<float>( time_ ) * 6.0f );
      float mod_i = ( 1.2f + ( math::sin( 4.0f + ftime * numbers::pi * 0.1f ) * 0.8f ) ) * 0.5f;
      float mod_j = 1.0f - math::cos( ftime * numbers::pi );
      float mod_g = ( ( ( mod_i * 0.5f ) * ( mod_i * mod_i ) + ( mod_j * mod_i * 0.5f ) * 0.5f ) * 3.0f );
      static const float ggg[8] = {
        0.0f, mod_g, 0.0f, 0.0f,
        0.0f, mod_g, 0.0f, 0.0f };
      simd::vec8f delta( static_cast<float>( ddelta ) );
      static const float s_gravity[8] = {
        0.0f, gravity_ ? -numbers::g : 0.0f, 0.0f, 0.0f,
        0.0f, gravity_ ? -numbers::g : 0.0f, 0.0f, 0.0f };
      simd::vec8f gravity( s_gravity );
      for ( size_t i = 0; i < ( c_particleCount / 8 ); ++i )
      {
        simd::vec8f mass_packed( &masses_[i * 8] );
        simd::vec8f mass[4];
        mass_packed.unpack8x4( mass[0], mass[1], mass[2], mass[3] );
        for ( size_t j = 0; j < 4; ++j )
        {
          size_t components_index = ( ( i * 8 ) + ( j * 2 ) );
          simd::vec8f acceleration( &acceleration_[components_index][0] );
          simd::vec8f forces = mass[j] * gravity;
          acceleration = acceleration + ( forces / mass[j] );
          simd::vec8f velocity( &velocities_[components_index][0] );
          velocity = simd::vec8f::fma( acceleration, delta, velocity );
          simd::vec8f position( &positions_[components_index][0] );
          position = position + velocity + ggg;
          position.storeNontemporal( &positions_[components_index][0] );
          velocity.storeNontemporal( &velocities_[components_index][0] );
        }
      }
    }
    void draw( shaders::Shaders& shaders, const Material& mat )
    {
      if ( !mat.uploaded() || mat.layers_.empty() || !mat.layers_[0].texture_ )
        return;
      buffer_->buffer().lock();
      auto points = buffer_->buffer().buffer().data();
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
    neko_avx_align vec3 axes_[c_particleCount];
    neko_avx_align float rot_[c_particleCount];
    aabb box_;
  public:
    SakuraSystem( const aabb& box );
    virtual void resetParticle( size_t index );
    virtual void update( GameTime delta );
  };

}