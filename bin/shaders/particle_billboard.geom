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
  vec3 size;
  vec4 color;
  float id;
} gs_in[];

out VertexData
{
  vec2 texcoord;
  vec4 color;
} gs_out;

void makeVertex( vec2 v, vec2 tc, vec4 position )
{
  mat4 projection = world.camera.projection;
  vec2 vv = position.xy + v;
  gl_Position = ( projection * vec4( vv, position.zw ) );
  gs_out.texcoord = tc;
  gs_out.color = gs_in[0].color;
  EmitVertex();
}

void main()
{
  mat4 view = world.camera.view;
  vec4 pos = view * gl_in[0].gl_Position;
  vec2 sz = gs_in[0].size.xy;

  makeVertex( vec2( -0.5, -0.5 ) * sz, vec2( 0.0, 0.0 ), pos );
  makeVertex( vec2(  0.5, -0.5 ) * sz, vec2( 1.0, 0.0 ), pos );
  makeVertex( vec2( -0.5,  0.5 ) * sz, vec2( 0.0, 1.0 ), pos );
  makeVertex( vec2(  0.5,  0.5 ) * sz, vec2( 1.0, 1.0 ), pos );

  EndPrimitive();
}