#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec2 texcoord;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2D tex;

void main()
{
  vec4 diffuse = texture( tex, interpolateAtSample( vs_out.texcoord, gl_SampleID ) ).rgba;
  out_color = diffuse;
}
