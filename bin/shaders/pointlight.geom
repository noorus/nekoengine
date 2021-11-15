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
  vec4 lightColor;
} gs_in[];

out VertexData
{
  vec2 texcoord;
  vec4 lightColor;
} gs_out;

const float c_size = 0.1;

void main()
{
  mat4 projection = world.camera.projection;
  float halfSize = ( c_size * 0.5 );

  gl_Position = projection * ( vec4( -halfSize, -halfSize, 0.0, 0.0 ) + gl_in[0].gl_Position );
  gs_out.texcoord = vec2( 0.0, 0.0 );
  gs_out.lightColor = gs_in[0].lightColor;
  EmitVertex();

  gl_Position = projection * ( vec4( halfSize, -halfSize, 0.0, 0.0 ) + gl_in[0].gl_Position );
  gs_out.texcoord = vec2( 1.0, 0.0 );
  gs_out.lightColor = gs_in[0].lightColor;
  EmitVertex();

  gl_Position = projection * ( vec4( -halfSize, halfSize, 0.0, 0.0 ) + gl_in[0].gl_Position );
  gs_out.texcoord = vec2( 0.0, 1.0 );
  gs_out.lightColor = gs_in[0].lightColor;
  EmitVertex();

  gl_Position = projection * ( vec4( halfSize, halfSize, 0.0, 0.0 ) + gl_in[0].gl_Position );
  gs_out.texcoord = vec2( 1.0, 1.0 );
  gs_out.lightColor = gs_in[0].lightColor;
  EmitVertex();

  EndPrimitive();
}