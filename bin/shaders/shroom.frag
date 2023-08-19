#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec3 normal;
  vec2 texcoord;
  vec3 fragpos;
  vec4 color;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2DArray tex;
uniform int tex_layer;

#include "inc.colorutils.glsl"

void main()
{
  vec3 tc = vec3( interpolateAtSample( vs_out.texcoord, gl_SampleID ), tex_layer );

  vec3 Lo = texture( tex, tc ).rgb;
  float alpha = texture( tex, tc ).a;

  vec3 ambient = processing.ambient.rgb * Lo;
  vec3 diffuse = ambient + Lo;

  out_color = vs_out.color * vec4( diffuse, alpha );
}