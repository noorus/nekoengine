#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec3 normal;
  vec2 texcoord;
  vec3 fragpos;
  vec4 color;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2D texture_layer0_diffuse;
uniform sampler2D texture_layer1_diffuse;
uniform sampler2D texture_layer2_diffuse;
uniform sampler2D texture_layer3_diffuse;
uniform vec2 texture_dimensions;
uniform float pixelscale;
uniform sampler2D texture_blendmap;

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
  vec2 layer_dimensions = vec2( 80.0, 80.0 );
  vec2 btc = pixeledSamplingWrapped( tc * texture_dimensions, texture_dimensions );
  vec4 b = texture( texture_blendmap, btc );
  vec3 r = vec3( 0.0, 0.0, 0.0 );

  float alpha = 0.0;
  
  vec2 stc = pixeledSamplingWrapped( tc * texture_dimensions, layer_dimensions );
  {
    vec3 diffuse = texture( texture_layer3_diffuse, stc ).rgb;
    r = mix( r, diffuse, b.a );
    alpha += b.a;
  }
  {
    vec3 diffuse = texture( texture_layer2_diffuse, stc ).rgb;
    r = mix( r, diffuse, b.b );
    alpha += b.b;
  }
  {
    vec3 diffuse = texture( texture_layer1_diffuse, stc ).rgb;
    r = mix( r, diffuse, b.g );
    alpha += b.g;
  }
  {
    vec3 diffuse = texture( texture_layer0_diffuse, stc ).rgb;
    r = mix( r, diffuse, b.r );
    alpha += b.r;
  }

  out_color = ( vs_out.color * clamp( vec4( r, alpha ), 0.0, 1.0 ) );
}