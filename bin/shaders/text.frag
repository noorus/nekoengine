#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec2 texcoord;
  vec3 fragpos;
  vec4 color;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2D tex;

#include "inc.colorutils.glsl"

void main()
{
  vec2 tc = interpolateAtSample( vs_out.texcoord, gl_SampleID );
  float alpha = texture( tex, tc ).r;
  vec4 color = vs_out.color.rgba;
  out_color = vec4( color.rgb, alpha * color.a );
}