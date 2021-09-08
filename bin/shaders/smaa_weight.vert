#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.smaa.glsl"

out gl_PerVertex{
  vec4 gl_Position;
};

uniform vec2 resolution;

layout ( location = 0 ) in vec2 vbo_position;
layout ( location = 1 ) in vec2 vbo_texcoord;

out vec2 out_texcoord;
out vec2 out_pixcoord;
out vec4 out_offset[3];

void main()
{
  vec4 SMAA_RT_METRICS = vec4( 1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y );

	out_texcoord = vec2( ( vbo_position + 1.0) / 2.0 );

  out_pixcoord = out_texcoord * SMAA_RT_METRICS.zw;

  // We will use these offsets for the searches later on (see @PSEUDO_GATHER4):
  out_offset[0] = mad( SMAA_RT_METRICS.xyxy, vec4( -0.25, -0.125,  1.25, -0.125 ), out_texcoord.xyxy );
  out_offset[1] = mad( SMAA_RT_METRICS.xyxy, vec4( -0.125, -0.25, -0.125,  1.25 ), out_texcoord.xyxy );

  // And these for the searches, they indicate the ends of the loops:
  out_offset[2] = mad(
    SMAA_RT_METRICS.xxyy,
    vec4( -2.0, 2.0, -2.0, 2.0) * float( SMAA_MAX_SEARCH_STEPS ),
    vec4( out_offset[0].xz, out_offset[1].yw )
  );

  gl_Position = vec4( vbo_position, 0.0, 1.0 );
}