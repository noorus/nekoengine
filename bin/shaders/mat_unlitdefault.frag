#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.buffers.glsl"

in VertexData {
  vec3 normal;
  vec2 texcoord;
  vec3 fragpos;
} vs_out;

layout ( location = 0 ) out vec4 out_color;
layout ( location = 1 ) out vec4 out_gbuffer;

uniform float gamma;
uniform sampler2D tex;

#include "inc.colorutils.glsl"

void main()
{
  vec2 tc = vec2( vs_out.texcoord.x, vs_out.texcoord.y );

  vec3 Lo = texture( tex, tc ).rgb;
  float alpha = texture( tex, tc ).a;

  vec3 ambient = processing.ambient.rgb * Lo;
  vec3 color = ambient + Lo;

  out_color = vec4( color, alpha );
  out_gbuffer = vec4( 0.0, 0.0, 0.0, alpha );
}