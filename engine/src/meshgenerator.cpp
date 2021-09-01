#include "stdafx.h"
#include "gfx_types.h"
#include "meshmanager.h"
#include "neko_exception.h"
#include "console.h"

namespace neko {

#pragma warning(push)
#pragma warning(disable: 4244)

  namespace util {

  inline void zeroNans( vec3& v )
  {
    if ( std::isnan( v.x ) || std::isnan( v.y ) || std::isnan( v.z ) )
      v = vec3( 0.0f );
  }

  inline void zeroNans( vec4& v )
  {
    if ( std::isnan( v.x ) || std::isnan( v.y ) || std::isnan( v.z ) || std::isnan( v.w ) )
      v = vec4( 0.0f );
  }

  /*void generateTangents( vector<Vertex3D>& verts, const vector<GLuint>& indices )
  {
    for ( GLuint l = 0; l < indices.size(); ++l )
      verts[indices[l]].tangent = vec4( 0.0f );

    int inconsistents = 0;
    for ( GLuint l = 0; l < indices.size(); ++l )
    {
      GLuint i = indices[l];
      GLuint j = indices[( l + 1 ) % 3 + l / 3 * 3];
      GLuint k = indices[( l + 2 ) % 3 + l / 3 * 3];
      vec3 n = verts[i].normal;
      vec3 v1 = verts[j].position - verts[i].position, v2 = verts[k].position - verts[i].position;
      vec2 t1 = verts[j].texcoord - verts[i].texcoord, t2 = verts[k].texcoord - verts[i].texcoord;

      // Is the texture flipped?
      float uv2xArea = t1.x * t2.y - t1.y * t2.x;
      if ( math::abs( uv2xArea ) < 0x1p-20 )
        continue; // Smaller than 1/2 pixel at 1024x1024
      float flip = uv2xArea > 0 ? 1 : -1;
      // 'flip' or '-flip'; depends on the handedness of the space.
      if ( verts[i].tangent.w != 0 && verts[i].tangent.w != -flip )
        inconsistents++;
      verts[i].tangent.w = -flip;

      // Project triangle onto tangent plane
      v1 -= n * math::dot( v1, n );
      v2 -= n * math::dot( v2, n );
      // Tangent is object space direction of texture coordinates
      vec3 s = math::normalize( ( t2.y * v1 - t1.y * v2 ) * flip );

      // Use angle between projected v1 and v2 as weight
      float angle = math::acos( math::dot( v1, v2 ) / ( math::length( v1 ) * math::length( v2 ) ) );
      verts[i].tangent += vec4( s * angle, 0 );
    }

    for ( GLuint l = 0; l < indices.size(); ++l )
    {
      vec4& t = verts[indices[l]].tangent;
      t = vec4( normalize( vec3( t.x, t.y, t.z ) ), t.w );
    }
  }*/

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
      zeroNans( vert.tangent );
      vert.bitangent = math::cross( n, xyz ) * w;
      zeroNans( vert.bitangent );
    }
  }

  }

  void implAddPlane( vector<Vertex3D>& verts, vector<GLuint>& indices, GLuint& offset, vec2 dimensions, vec2u segments, vec3 normal, vec4 color, vec3 position = vec3( 0.0f ) )
  {
    verts.resize( verts.size() + ( ( segments.x + 1 ) * ( segments.y + 1 ) ) );
    indices.resize( indices.size() + ( segments.x * segments.y * 6 ) );

    auto vx = glm::perp( vec3( 1.0f, 0.0f, 0.0f ), normal );
    auto vy = glm::cross( normal, vx );

    auto delta1 = vec3( dimensions.x / (Real)segments.x * vx );
    auto delta2 = vec3( dimensions.y / (Real)segments.y * vy );

    auto orig = position + vec3( -0.5f * dimensions.x * vx - 0.5f * dimensions.y * vy );
    for ( auto i = 0; i <= segments.x; ++i )
      for ( auto j = 0; j <= segments.y; ++j )
      {
        auto v = Vertex3D{
          orig + (Real)i * delta1 + (Real)j * delta2,
          normal,
          vec2( (Real)i, (Real)j ), // vec2( i / (Real)segments.x, j / (Real)segments.y ),
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

  pair<vector<Vertex3D>, vector<GLuint>> MeshGenerator::makePlane( vec2 dimensions, vec2u segments, vec3 normal, vec4 color )
  {
    vector<Vertex3D> verts;
    vector<GLuint> indices;
    GLuint offset = 0;

    implAddPlane( verts, indices, offset, dimensions, segments, normal, color );

    return make_pair( verts, indices );
  }

  pair<vector<Vertex3D>, vector<GLuint>> MeshGenerator::makeSphere( Real diameter, vec2u segments, vec4 color )
  {
    vector<Vertex3D> verts;
    vector<GLuint> indices;

    auto scaler = vec3( diameter );

    for ( unsigned int y = 0; y <= segments.y; ++y )
    {
      for ( unsigned int x = 0; x <= segments.x; ++x )
      {
        vec2 fsegm( static_cast<Real>( x ), static_cast<Real>( y ) );
        fsegm /= segments;
        vec3 pos(
          math::cos( fsegm.x * 2.0f * numbers::pi ) * math::sin( fsegm.y * numbers::pi ),
          math::cos( fsegm.y * numbers::pi ),
          math::sin( fsegm.x * 2.0f * numbers::pi ) * math::sin( fsegm.y * numbers::pi )
        );
        verts.emplace_back( pos * scaler, pos, fsegm, color );
      }
    }

    auto odd = false;
    for ( unsigned int y = 0; y < segments.y; ++y )
    {
      if ( !odd )
      {
        for ( unsigned int x = 0; x <= segments.x; ++x )
        {
          indices.push_back( ( y + 1 ) * ( segments.x + 1 ) + x );
          indices.push_back( y * ( segments.x + 1 ) + x );
        }
      }
      else
      {
        for ( auto x = (int)segments.x; x >= 0; --x )
        {
          indices.push_back( y * ( segments.x + 1 ) + x );
          indices.push_back( ( y + 1 ) * ( segments.x + 1 ) + x );
        }
      }
      odd = !odd;
    }

    return make_pair( verts, indices );
  }

#pragma warning(pop)

}