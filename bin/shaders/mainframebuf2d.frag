#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec2 texcoord;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2D texMain;

uniform float gamma;
uniform float exposure;

#define COLORUTILS_TONEMAPPING
#include "inc.colorutils.glsl"

void main()
{
  vec3 hdrcolor = texture( texMain, vs_out.texcoord ).rgb;

  vec3 result = tonemap_linear( hdrcolor );

  out_color = vec4( result, 1.0 );
}
