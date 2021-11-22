#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec2 texcoord;
  vec4 color;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2D tex;

void main()
{
  vec4 diffuse = texture( tex, vs_out.texcoord );
  if ( diffuse.a < 0.1 )
    discard;
  out_color = diffuse;
}