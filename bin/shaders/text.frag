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

#include "inc.colorutils.glsl"

void main()
{
  vec2 tc = vec2( vs_out.texcoord.x, vs_out.texcoord.y );
  float alpha = texture( tex, tc ).r;
  vec4 color = vs_out.color.rgba;
  out_color = vec4( color.rgb, alpha * color.a );
}