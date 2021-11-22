#version 450 core

#include "inc.buffers.glsl"

layout ( points ) in;
layout ( triangle_strip, max_vertices = 4 ) out;

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
  vec2 size;
  vec4 color;
} gs_in[];

out VertexData
{
  vec2 texcoord;
  vec4 color;
} gs_out;

uniform mat4 model;

vec3 qrot( vec4 q, vec3 v )
{
  float z = v.z;
  return ( v.xyz + 2.0 * cross( q.xyz, cross( q.xyz, v ) + q.w * v ) );
}

void makeVertex( vec3 bbin, vec2 tc, vec4 position )
{
  mat4 projection = world.camera.projection;
  mat4 view = world.camera.view;
  vec3 translated = position.xyz + bbin;
  gl_Position = ( projection * view * model * vec4( translated, position.w ) );
  gs_out.texcoord = tc;
  gs_out.color = gs_in[0].color;
  EmitVertex();
}

void main()
{
  vec4 pos = gl_in[0].gl_Position;
  vec3 sz = vec3( gs_in[0].size, 0.0 );
  vec4 orientation = gs_in[0].orientation;
  
  makeVertex( qrot( orientation, vec3( -0.5, -0.5, 0.0 ) * sz ), vec2( 0.0, 0.0 ), pos );
  makeVertex( qrot( orientation, vec3(  0.5, -0.5, 0.0 ) * sz ), vec2( 1.0, 0.0 ), pos );
  makeVertex( qrot( orientation, vec3( -0.5,  0.5, 0.0 ) * sz ), vec2( 0.0, 1.0 ), pos );
  makeVertex( qrot( orientation, vec3(  0.5,  0.5, 0.0 ) * sz ), vec2( 1.0, 1.0 ), pos );

  EndPrimitive();
}