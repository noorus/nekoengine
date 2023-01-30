#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec2 texcoord;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2D tex;

#define COLORUTILS_TONEMAPPING
#include "inc.colorutils.glsl"

void main()
{
  vec3 hdrcolor = texture( tex, interpolateAtSample( vs_out.texcoord, gl_SampleID ) ).rgb;

  vec3 result = tonemap_linear( hdrcolor );
  out_color = vec4( result.rgb, 1.0 );
}
