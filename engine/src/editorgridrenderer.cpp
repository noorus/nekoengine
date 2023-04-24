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
#include "text.h"
#include "filesystem.h"
#include "specialrenderers.h"

namespace neko {

  using namespace gl;

  EditorGridRenderer::EditorGridRenderer()
  {
    viz_ = make_unique<LineRenderBuffer<1024>>();
  }

  EditorGridRenderer::~EditorGridRenderer()
  {
    viz_.reset();
  }

  void EditorGridRenderer::update( EditorOrthoCamera& camera )
  {
    auto aspect = camera.aspect();
    auto normal = math::normalize( -camera.direction() );
    auto origin = camera.position() + ( ( camera.far() - 1.0f ) * camera.direction() );
    auto color = vec4( 0.3f, 0.3f, 0.3f, 1.0f );

    constexpr int count = 10;
    auto verts = viz_->buffer().lock();

    for ( size_t i = 0; i < 44; ++i )
      verts[i].color = color;

    vec2i segments { count, count };
    vec2 dimensions { (Real)segments.x, (Real)segments.y };

    auto vx = math::perpendicular( normal );
    auto vy = glm::cross( normal, vx );
    auto delta1 = vec3( dimensions.x / (Real)segments.x * vx );
    auto delta2 = vec3( dimensions.y / (Real)segments.y * vy );

    size_t i = 0;
    auto fit = vec3( math::floor( origin.x ), math::floor( origin.y ), math::floor( origin.y ) );
    auto orig = fit + vec3( -0.5f * dimensions.x * vx - 0.5f * dimensions.y * vy );
    for ( auto x = 0; x <= segments.x; ++x )
    {
      verts[i++].pos = orig + (Real)x * delta1;
      verts[i++].pos = orig + (Real)x * delta1 + dimensions.y * delta2;
    }
    for ( auto y = 0; y <= segments.y; ++y )
    {
      verts[i++].pos = orig + (Real)y * delta2;
      verts[i++].pos = orig + dimensions.x * delta1 + (Real)y * delta2;
    }

    viz_->buffer().unlock();
  }

  void EditorGridRenderer::draw( shaders::Shaders& shaders )
  {
    glLineWidth( 2.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthMask( GL_FALSE );
    glEnable( GL_LINE_SMOOTH );
    auto pl = &shaders.usePipeline( "dbg_line" );
    viz_->draw( *pl, 44, 0, gl::GL_LINES );
  }

}