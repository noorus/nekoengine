#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec2 texcoord;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2D tex;

float depthLinearize( float depth )
{
  float near = world.camera.nearDist;
  float far = world.camera.farDist;
  return ( 2.0 * near ) / ( far + near - depth * ( far - near ) );
}

void main()
{
  float value = depthLinearize( texture( tex, vs_out.texcoord ).r );
  out_color = vec4( vec3( value ), 1.0 );
}
