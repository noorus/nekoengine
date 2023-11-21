#include "pch.h"
#include "gfx_types.h"
#include "renderer.h"
#include "mesh_primitives.h"
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
#include "specialrenderers.h"

namespace neko {

  using namespace gl;

  AxesPointerRenderer::AxesPointerRenderer()
  {
    viz_ = make_unique<LineRenderBuffer<6>>();
  }

  AxesPointerRenderer::~AxesPointerRenderer()
  {
    viz_.reset();
  }

  void AxesPointerRenderer::draw( Shaders& shaders, vec3 origin, vec3 up, vec3 right )
  {
    auto verts = viz_->buffer().lock();
    Real length = 0.5f;
    verts[0].color = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
    verts[0].pos = origin;
    verts[1].color = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
    verts[1].pos = origin + ( right * length );
    verts[2].color = vec4( 0.0f, 1.0f, 0.0f, 1.0f );
    verts[2].pos = origin;
    verts[3].color = vec4( 0.0f, 1.0f, 0.0f, 1.0f );
    verts[3].pos = origin + ( up * length );
    verts[4].color = vec4( 0.0f, 0.0f, 1.0f, 1.0f );
    verts[4].pos = origin;
    verts[5].color = vec4( 0.0f, 0.0f, 1.0f, 1.0f );
    verts[5].pos = origin + ( math::cross( right, up ) * length );
    viz_->buffer().unlock();
    glLineWidth( 3.0f );
    glDisable( GL_DEPTH_TEST );
    glDepthMask( GL_FALSE );
    glDisable( GL_LINE_SMOOTH );
    auto pl = &shaders.usePipeline( "dbg_line" );
    viz_->draw( *pl, 6, 0, gl::GL_LINES );
  }

  EditorGridRenderer::EditorGridRenderer()
  {
    viz_ = make_unique<LineRenderBuffer<1024>>();
  }

  EditorGridRenderer::~EditorGridRenderer()
  {
    viz_.reset();
  }

  void EditorGridRenderer::update( const EditorViewport& viewport, EditorOrthoCamera& camera )
  {
    if ( camera.frustum().type() != Projection_Orthographic )
      return;

    if ( camera.frustum().radius() < 1.0f )
    {
      drawCount_ = 0;
      return;
    }

    auto area = math::ceil( camera.frustum().aspect() * camera.frustum().radius() );
    auto normal = math::normalize( -camera.direction() );
    auto& origin = camera.position(); // viewport.ndcPointToWorld( { 0.0f, 0.0f, 0.0f } );

    auto color = viewport.drawopGridColor();

    int count = math::iround( area ) + 1;
    auto verts = viz_->buffer().lock();

    drawCount_ = math::min( count * 4 + 4, 1024 );

    for ( size_t i = 0; i < drawCount_; ++i )
      verts[i].color = color;

    vec2i segments { count, count };
    vec2 dimensions { (Real)segments.x, (Real)segments.y };

    auto vx = math::perpendicular( normal );
    auto vy = glm::cross( normal, vx );

    auto delta1 = vec3( dimensions.x / (Real)segments.x * vx );
    auto delta2 = vec3( dimensions.y / (Real)segments.y * vy );

    size_t i = 0;
    auto fit = vec3( math::floor( origin.x ), math::floor( origin.y ), math::floor( origin.z ) );
    auto orig = fit + vec3( -0.5f * dimensions.x * vx - 0.5f * dimensions.y * vy );
    for ( auto x = 0; x <= segments.x; ++x )
    {
      verts[i++].pos = orig + (Real)x * delta1;
      if ( i >= drawCount_ )
        break;
      verts[i++].pos = orig + (Real)x * delta1 + dimensions.y * delta2;
      if ( i >= drawCount_ )
        break;
    }
    for ( auto y = 0; y <= segments.y; ++y )
    {
      verts[i++].pos = orig + (Real)y * delta2;
      if ( i >= drawCount_ )
        break;
      verts[i++].pos = orig + dimensions.x * delta1 + (Real)y * delta2;
      if ( i >= drawCount_ )
        break;
    }

    viz_->buffer().unlock();
  }

  void EditorGridRenderer::draw( Shaders& shaders )
  {
    if ( !drawCount_ )
      return;

    glLineWidth( 1.0f );
    glDisable( GL_DEPTH_TEST );
    glDepthMask( GL_FALSE );
    glDisable( GL_LINE_SMOOTH );
    auto pl = &shaders.usePipeline( "editor_bgline" );
    // pl->setUniform( "model", mat4(1.0f) );
    viz_->draw( *pl, drawCount_, 0, gl::GL_LINES );
  }

}