#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.smaa.glsl"

uniform vec2 resolution;

uniform sampler2D albedo_tex;
uniform sampler2D blend_tex;

in vec2 in_texcoord;
in vec4 in_offset;

out vec4 out_color;

vec4 SMAA_RT_METRICS = vec4( 1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y );

void main()
{
  // Fetch the blending weights for current pixel:
  vec4 a;
  a.x = texture( blend_tex, in_offset.xy ).a; // Right
  a.y = texture( blend_tex, in_offset.zw ).g; // Top
  a.wz = texture( blend_tex, in_texcoord ).xz; // Bottom / Left

  // Is there any blending weight with a value greater than 0.0?
  if ( dot( a, vec4( 1.0, 1.0, 1.0, 1.0 ) ) <= 1e-5 )
  {
    out_color = texture( albedo_tex, in_texcoord ); // LinearSampler
  }
  else
  {
    bool h = max( a.x, a.z ) > max( a.y, a.w ); // max(horizontal) > max(vertical)

    // Calculate the blending offsets:
    vec4 blendingOffset = vec4( 0.0, a.y, 0.0, a.w );
    vec2 blendingWeight = a.yw;
    SMAAMovc( bvec4( h, h, h, h ), blendingOffset, vec4( a.x, 0.0, a.z, 0.0 ) );
    SMAAMovc( bvec2( h, h ), blendingWeight, a.xz );
    blendingWeight /= dot( blendingWeight, vec2( 1.0 ) );

    // Calculate the texture coordinates:
    vec4 blendingCoord = mad( blendingOffset, vec4( SMAA_RT_METRICS.xy, -SMAA_RT_METRICS.xy ), in_texcoord.xyxy );

    // We exploit bilinear filtering to mix current pixel with the chosen neighbor:
    out_color = blendingWeight.x * texture( albedo_tex, blendingCoord.xy );
    out_color += blendingWeight.y * texture( albedo_tex, blendingCoord.zw );
  }
}