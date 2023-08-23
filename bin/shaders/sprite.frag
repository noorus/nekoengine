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
uniform vec2 tex_dimensions;

#include "inc.colorutils.glsl"

void main()
{
  vec2 d = 0.7 * vec2( dFdx( vs_out.texcoord.x ), dFdy( vs_out.texcoord.y ) );
  vec2 fx = fract( vs_out.texcoord );
  vec2 fc = clamp( 0.5 / d * fx, 0.0, 0.5 ) + clamp( 0.5 / d * ( fx - 1.0 ) + 0.5, 0.0, 0.5 );

  // vec3 tc = vec3( interpolateAtSample( vs_out.texcoord, gl_SampleID ), tex_layer );
  vec3 tc = vec3( ( floor( vs_out.texcoord ) + fc ) / tex_dimensions, tex_layer );

  vec3 Lo = texture( tex, tc ).rgb;
  float alpha = texture( tex, tc ).a;

  //vec3 ambient = processing.ambient.rgb * Lo;
  //vec3 diffuse = ambient + Lo;
  vec3 diffuse = Lo;

  out_color = vs_out.color * vec4( diffuse, alpha );
}
