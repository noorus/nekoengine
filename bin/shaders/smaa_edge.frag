#version 450 core

#include "inc.smaa.glsl"

uniform sampler2D albedo_tex;

in vec2 in_texcoord;
in vec4 in_offset[3];

out vec4 out_color;

void main()
{
  // Calculate the threshold:
  vec2 threshold = vec2( SMAA_THRESHOLD );

  // Calculate color deltas:
  vec4 delta;
  vec3 c = texture( albedo_tex, in_texcoord ).rgb;

  vec3 cLeft = texture( albedo_tex, in_offset[0].xy ).rgb;
  vec3 t = abs( c - cLeft );
  delta.x = max( max( t.r, t.g ), t.b );

  vec3 cTop  = texture( albedo_tex, in_offset[0].zw ).rgb;
  t = abs( c - cTop );
  delta.y = max( max( t.r, t.g ), t.b );

  // We do the usual threshold:
  vec2 edges = step( threshold, delta.xy );

  // Then discard if there is no edge:
  if ( dot( edges, vec2( 1.0, 1.0 ) ) == 0.0 )
    discard;

  // Calculate right and bottom deltas:
  vec3 cRight = texture( albedo_tex, in_offset[1].xy ).rgb;
  t = abs( c - cRight );
  delta.z = max( max( t.r, t.g ), t.b );

  vec3 cBottom  = texture( albedo_tex, in_offset[1].zw ).rgb;
  t = abs( c - cBottom );
  delta.w = max( max( t.r, t.g ), t.b );

  // Calculate the maximum delta in the direct neighborhood:
  vec2 maxDelta = max( delta.xy, delta.zw );

  // Calculate left-left and top-top deltas:
  vec3 cLeftLeft  = texture( albedo_tex, in_offset[2].xy ).rgb;
  t = abs( c - cLeftLeft );
  delta.z = max( max( t.r, t.g ), t.b );

  vec3 cTopTop = texture( albedo_tex, in_offset[2].zw ).rgb;
  t = abs( c - cTopTop );
  delta.w = max( max( t.r, t.g ), t.b );

  // Calculate the final maximum delta:
  maxDelta = max( maxDelta.xy, delta.zw );
  float finalDelta = max( maxDelta.x, maxDelta.y );

  // Local contrast adaptation:
  edges.xy *= step( finalDelta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy );

  out_color = vec4( edges, 0.0, 1.0 );
}