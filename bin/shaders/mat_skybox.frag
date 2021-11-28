#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec3 normal;
  vec2 texcoord;
  vec3 fragpos;
  vec4 color;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2D tex;

#include "inc.math.glsl"

void main()
{
  vec2 tc = util_posToSphericalUV( normalize( vs_out.fragpos ) );
  vec3 Lo = texture( tex, tc * vec2( 1, -1 ) ).rgb;

  vec3 ambient = processing.ambient.rgb * Lo;
  vec3 diffuse = ambient + Lo;

  out_color = vec4( diffuse, 1.0 );
}