#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec3 normal;
  vec2 texcoord;
  vec3 fragpos;
  vec4 color;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform float gamma;
uniform sampler2D tex;

#include "inc.colorutils.glsl"

void main()
{
  vec2 tc = vec2( vs_out.texcoord.x, vs_out.texcoord.y );

  vec3 Lo = texture( tex, tc ).rgb;
  float alpha = texture( tex, tc ).a;

  vec3 ambient = processing.ambient.rgb * Lo;
  vec3 diffuse = ambient + Lo;

  out_color = vs_out.color * vec4( diffuse, alpha );
}