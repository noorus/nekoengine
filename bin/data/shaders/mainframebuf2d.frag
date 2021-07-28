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

#include "inc.colorutils.glsl"

vec3 gaussianBlurredSample( sampler2D txtr, vec2 coord )
{
  const int ksize = ( c_gaussianBlurSamples - 1 ) / 2;

  vec2 texelSize = 1.0 / textureSize( txtr, 0 );

  vec3 color = vec3( 0.0 );
  for ( int i = -ksize; i <= ksize; ++i )
  {
    for ( int j = -ksize; j <= ksize; ++j )
    {
      vec2 samplecoord = coord + ( vec2( float( i ), float( j ) ) * texelSize );
      color += processing.gaussianKernel[ksize + j]
        * processing.gaussianKernel[ksize + i]
        * texture( txtr, samplecoord ).rgb;
    }
  }

  return ( color / ( processing.gaussianZ * processing.gaussianZ ) );
}

void main()
{
  vec3 hdrcolor = texture( texMain, texcoord ).rgb;

  vec3 result = vec3( 0.0 );
  if ( hdr )
  {
    vec3 bloomcolor = gaussianBlurredSample( texGBuffer, texcoord );
    hdrcolor += bloomcolor;
    result = tonemap_linear( hdrcolor );
  }
  else
  {
    result = pow( hdrcolor, vec3( 1.0 / gamma ) );
  }
  out_color = vec4( result, 1.0 );
}
