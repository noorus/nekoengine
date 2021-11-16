#version 450 core

#include "inc.smaa.glsl"

out gl_PerVertex{
  vec4 gl_Position;
};

uniform vec2 resolution;

layout ( location = 0 ) in vec2 vbo_position;
layout ( location = 1 ) in vec2 vbo_texcoord;

out vec2 out_texcoord;
out vec4 out_offset[3];

void main()
{
  vec4 SMAA_RT_METRICS = vec4( 1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y );

	out_texcoord = vec2( ( vbo_position + 1.0 ) / 2.0 );

  out_offset[0] = mad( SMAA_RT_METRICS.xyxy, vec4( -1.0, 0.0, 0.0, -1.0 ), out_texcoord.xyxy );
  out_offset[1] = mad( SMAA_RT_METRICS.xyxy, vec4(  1.0, 0.0, 0.0,  1.0 ), out_texcoord.xyxy );
  out_offset[2] = mad( SMAA_RT_METRICS.xyxy, vec4( -2.0, 0.0, 0.0, -2.0 ), out_texcoord.xyxy );

  gl_Position = vec4( vbo_position, 0.0, 1.0 );
}