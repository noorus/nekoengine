#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec3 normal;
  vec2 texcoord;
  vec3 fragpos;
  vec4 color;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2D diffuse_tex;
uniform vec2 diffuse_dimensions;
uniform sampler2D blendmap_tex;
uniform vec2 blendmap_dimensions;
uniform float pixelscale;

#include "inc.colorutils.glsl"

vec2 pixeledSamplingWrapped( vec2 tc, vec2 texdims )
{
  vec2 d = 0.7 * vec2( dFdx( tc.x ), dFdy( tc.y ) );
  vec2 fx = fract( tc );
  vec2 fc = clamp( 0.5 / d * fx, 0.0, 0.5 ) + clamp( 0.5 / d * ( fx - 1.0 ) + 0.5, 0.0, 0.5 );
  return fract( vec2( ( floor( tc ) + fc ) / texdims ) ); // <- remove fract to not wrap around UV
}

void main()
{
  vec2 tc = interpolateAtSample( vs_out.texcoord, gl_SampleID );
  vec2 btc = pixeledSamplingWrapped( tc * blendmap_dimensions, blendmap_dimensions );
  vec4 blend = texture( blendmap_tex, btc );
  vec3 r = vec3( 0.0, 0.0, 0.0 );

  float alpha = 0.0;
  
  vec2 stc = pixeledSamplingWrapped( tc * blendmap_dimensions, diffuse_dimensions );

  {
    r += texture( diffuse_tex, stc ).rgb;
    alpha += blend.r;
  }

  out_color = ( vs_out.color * clamp( vec4( r, alpha ), 0.0, 1.0 ) );
}