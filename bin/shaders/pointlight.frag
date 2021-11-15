#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec2 texcoord;
  vec4 lightColor;
} vs_out;

layout ( location = 0 ) out vec4 out_color;
layout ( location = 1 ) out vec4 out_gbuffer;

void main()
{
  float limiter = ( vs_out.lightColor.r + vs_out.lightColor.g + vs_out.lightColor.b );
  vec4 color = vec4( vs_out.lightColor.rgb / vec3( limiter ), 1.0 );
  out_color = color;
  out_gbuffer = color;
}