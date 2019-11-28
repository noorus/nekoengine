#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"
#include "console.h"

namespace neko {

  void MeshGenerator::makePlane( DynamicMesh& mesh, vec2 dimensions, vec2u segments, vec3 normal )
  {
    vector<Vertex3D> verts;
    verts.resize( ( segments.x + 1 ) * ( segments.y + 1 ) );
    vector<GLuint> indices;
    indices.resize( segments.x * segments.y * 6 );

    auto vx = glm::perp( vec3( 1.0f, 0.0f, 0.0f ), normal );
    auto vy = glm::cross( normal, vx );

    auto delta1 = vec3( dimensions.x / (Real)segments.x * vx );
    auto delta2 = vec3( dimensions.y / (Real)segments.y * vy );

    auto orig = vec3( -0.5f * dimensions.x * vx - 0.5f * dimensions.y * vy );
    for ( auto i = 0; i <= segments.x; ++i )
      for ( auto j = 0; j <= segments.y; ++j )
      {
        auto v = Vertex3D{
          orig + (Real)i * delta1 + (Real)j * delta2, normal,
          vec2( i / (Real)segments.x, j / (Real)segments.y ) };
        verts[i * ( segments.y + 1 ) + j] = move( v );
      }

    bool reverse = ( glm::dot( glm::cross( delta1, delta2 ), normal ) > 0.0f );

    GLuint offset = 0;
    for ( auto i = 0; i < segments.x; ++i )
    {
      for ( auto j = 0; j < segments.y; ++j )
      {
        auto base = ( i * segments.y + j ) * 6;
        indices[base + 0] = offset;
        indices[base + 1] = offset + ( reverse ? segments.y + 1 : 1 );
        indices[base + 2] = offset + ( reverse ? 1 : segments.y + 1 );
        indices[base + 3] = offset + 1;
        indices[base + 4] = offset + segments.y + ( reverse ? 1 : 2 );
        indices[base + 5] = offset + segments.y + ( reverse ? 2 : 1 );
        offset++;
      }
      offset++;
    }

    mesh.append( verts, indices );
  }

}