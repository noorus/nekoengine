#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec4 color;
  vec2 texcoord;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2D tex;

void main()
{
  vec4 diffuse = texture( tex, vs_out.texcoord );
  out_color = vec4( diffuse.b, diffuse.g, diffuse.r, diffuse.a ) * vs_out.color;
}