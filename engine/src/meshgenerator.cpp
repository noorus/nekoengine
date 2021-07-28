#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"
#include "console.h"

namespace neko {

#pragma warning(push)
#pragma warning(disable: 4244)

  namespace util {

  void generateTangentsAndBitangents( vector<Vertex3D>& verts, const vector<GLuint>& indices )
  {
    for ( auto& vert : verts )
    {
      vert.tangent = vec4( 0.0f );
      vert.bitangent = vec3( 0.0f );
    }

    int i = 0;
    while ( i < indices.size() )
    {
      auto& v0 = verts[indices[i]];
      auto& v1 = verts[indices[i + 1]];
      auto& v2 = verts[indices[i + 2]];

      vec3 e1 = v1.position - v0.position, e2 = v2.position - v0.position;
      float x1 = v1.texcoord.x - v0.texcoord.x, x2 = v2.texcoord.x - v0.texcoord.x;
      float y1 = v1.texcoord.y - v0.texcoord.y, y2 = v2.texcoord.y - v0.texcoord.y;
      float r = 1.0f / ( x1 * y2 - x2 * y1 );
      vec3 t = ( e1 * y2 - e2 * y1 ) * r;
      vec3 b = ( e2 * x1 - e1 * x2 ) * r;

      for ( int j = 0; j < 3; j++ )
      {
        if ( i + j >= verts.size() )
          break;
        verts[i + j].tangent += vec4( t, 0.0f );
        verts[i + j].bitangent += b;
      }

      i += 3;
    }

    for ( auto& vert : verts )
    {
      auto n = vec3( vert.normal );
      auto t = vec3( vert.tangent );
      auto b = vec3( vert.bitangent );
      auto xyz = math::normalize( math::rejection( t, n ) );
      auto w = ( math::dot( math::cross( t, b ), n ) > 0.0f ) ? 1.0f : -1.0f;
      vert.tangent = vec4( xyz, w );
      vert.bitangent = math::cross( n, xyz ) * w;
    }
  }

  }

  void implAddPlane( vector<Vertex3D>& verts, vector<GLuint>& indices, GLuint& offset, vec2 dimensions, vec2u segments, vec3 normal, vec3 position = vec3( 0.0f ) )
  {
    verts.resize( verts.size() + ( ( segments.x + 1 ) * ( segments.y + 1 ) ) );
    indices.resize( indices.size() + ( segments.x * segments.y * 6 ) );

    auto vx = glm::perp( vec3( 1.0f, 0.0f, 0.0f ), normal );
    auto vy = glm::cross( normal, vx );

    auto delta1 = vec3( dimensions.x / (Real)segments.x * vx );
    auto delta2 = vec3( dimensions.y / (Real)segments.y * vy );

    // World is a bleak place
    auto color = vec4( 1.0f, 1.0f, 1.0f, 1.0f );

    auto orig = position + vec3( -0.5f * dimensions.x * vx - 0.5f * dimensions.y * vy );
    for ( auto i = 0; i <= segments.x; ++i )
      for ( auto j = 0; j <= segments.y; ++j )
      {
        auto v = Vertex3D{
          orig + (Real)i * delta1 + (Real)j * delta2, normal,
          vec2( i / (Real)segments.x, j / (Real)segments.y ),
          color };
        verts[i * ( segments.y + 1 ) + j] = move( v );
      }

    bool reverse = ( glm::dot( glm::cross( delta1, delta2 ), normal ) > 0.0f );

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
  }

  pair<vector<Vertex3D>, vector<GLuint>> MeshGenerator::makePlane( vec2 dimensions, vec2u segments, vec3 normal )
  {
    vector<Vertex3D> verts;
    vector<GLuint> indices;
    GLuint offset = 0;

    implAddPlane( verts, indices, offset, dimensions, segments, normal );

    return make_pair( verts, indices );
  }

#pragma warning(pop)

}