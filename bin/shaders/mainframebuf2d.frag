#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.buffers.glsl"

uniform sampler2D texMain;
uniform sampler2D texGBuffer;

uniform bool hdr;
uniform float gamma;
uniform float exposure;

in vec2 texcoord;

out vec4 out_color;

#define COLORUTILS_TONEMAPPING
#include "inc.colorutils.glsl"

void main()
{
  vec3 hdrcolor = texture( texMain, texcoord ).rgb;

  vec3 result = vec3( 0.0 );
  if ( hdr )
  {
    vec3 bloomcolor = texture( texGBuffer, texcoord ).rgb;
    hdrcolor += bloomcolor;
    result = tonemap_linear( hdrcolor );
  }
  else
  {
    result = pow( hdrcolor, vec3( 1.0 / gamma ) );
  }
  out_color = vec4( result, 1.0 );
}
