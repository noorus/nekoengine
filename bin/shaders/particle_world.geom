#version 450 core

#include "inc.buffers.glsl"
#include "inc.math.glsl"

#define SUBDIVS 5

layout ( points ) in;
// Use separate invocations to draw each strip;
// For whatever reason multiple primitives per invocation does not work on Intel
layout ( invocations = SUBDIVS - 1 ) in;
layout ( triangle_strip, max_vertices = SUBDIVS * 2 ) out;

in gl_PerVertex
{
  vec4 gl_Position;
} gl_in[];

out gl_PerVertex
{
  vec4 gl_Position;
};

in VertexData
{
  vec4 orientation;
  vec3 size;
  vec4 color;
  float id;
} gs_in[];

out VertexData
{
  vec2 texcoord;
  vec4 color;
} gs_out;

uniform mat4 model;

void makeVertex( int stream, vec3 v, vec2 tc, vec4 position )
{
  mat4 projection = world.camera.projection;
  mat4 view = world.camera.view;
  vec3 translated = position.xyz + v;
  gl_Position = ( projection * view * model * vec4( translated, position.w ) );
  gs_out.texcoord = tc;
  gs_out.color = gs_in[0].color;
  EmitVertex();
}

vec3 verts[SUBDIVS * SUBDIVS];
vec2 uvs[SUBDIVS * SUBDIVS];

void makeStrip( int stream, vec3 size, vec4 orientation, vec4 origin )
{
  for ( int i = 0; i < ( SUBDIVS * 2 ); ++i )
  {
    int index = ( stream * SUBDIVS ) + ( mod( i, 2 ) < 1 ? ( i / 2 ) : SUBDIVS + ( ( i - 1 ) / 2 ) );
    makeVertex( stream, quat_multiply( orientation, verts[index] * size ), uvs[index], origin );
  }
  EndPrimitive();
}

float demorph( float x, float y )
{
  float my_seed = ( M_PI_4 + ( gs_in[0].id / 3.1 ) );
  float attn = ( 1.0 + cos( my_seed / 5.0 ) ) * 0.5;
  float dfn = cos( ( my_seed + x ) * M_PI_4 );
  return dfn + sin( ( my_seed + y ) * attn * M_PI ) * ( 0.1 + ( attn * 0.4 ) );
}

void main()
{
  vec4 pos = gl_in[0].gl_Position;
  vec4 orientation = gs_in[0].orientation;

  float step = 1.0 / float( SUBDIVS - 1 );
  for ( int j = 0; j < SUBDIVS; ++j )
  {
    for ( int i = 0; i < SUBDIVS; ++i )
    {
      int index = ( j * SUBDIVS ) + i;
      vec2 uv = vec2( float( i ) * step, float( j ) * step );
      uvs[index] = uv;
      verts[index] = vec3( uv - vec2( 0.5, 0.5 ), demorph( float( i ) * step, float( j ) * step ) );
    }
  }

  makeStrip( gl_InvocationID, gs_in[0].size, orientation, pos );
}