#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec2 texcoord;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

#include "inc.colorutils.glsl"
#include "inc.math.glsl"

vec3 rotate(vec3 p, float angle, vec3 axis)
{
    vec3 a = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float r = 1.0 - c;
    mat3 m = mat3(
              a.x * a.x * r + c,  a.y * a.x * r + a.z * s,  a.z * a.x * r - a.y * s,
        a.x * a.y * r - a.z * s,        a.y * a.y * r + c,  a.z * a.y * r + a.x * s,
        a.x * a.z * r + a.y * s,  a.y * a.z * r - a.x * s,  a.z * a.z * r + c
    );
    return m * p;
}

float circletest( vec2 pos, float scale, vec2 tc )
{
  pos = fract( pos * scale );
  pos = vec2( 0.5 ) - pos;
  float circle = 0.0; // sdf_circle( pos );
  float square = sdf_square( pos );
  float heart = sdf_heart( pos, 1.0 );
  //return mix( circle, square, tc.x );
  int pair = int( floor( mod( world.time, 3.0 ) ) );
  float a = smoothstep( 0.2, 0.7, mod( world.time, 1.0 ) );
  if ( pair == 0 )
    return mix( circle, square, a );
  if ( pair == 1 )
    return mix( square, heart, a );
  else
    return mix( heart, circle, a );
}

vec4 render( vec2 pos, float scale, vec2 tc )
{
  vec3 bgone = vec3( 246.0 / 255.0, 233.0 / 255.0, 210.0 / 255.0 );
  vec3 bgtwo = vec3( 251.0 / 255.0, 210.0 / 255.0, 210.0 / 255.0 );
  vec3 foreone = vec3( 255.0 / 255.0, 136.0 / 255.0, 192.0 / 255.0 );
  vec3 foretwo = vec3( 255.0 / 255.0, 59.0 / 255.0, 150.0 / 255.0 );
  float dt = ( smoothstep( 0.0, 1.2 + ( sin( world.time * 0.6 ) * 0.6 ), tc.x ) * 0.8 );
  float multip = 1.0 - smoothstep( 0.099, 0.105, ( 0.2 + dt ) * circletest( pos, scale, tc ) );
  return vec4( mix( mix( bgone, bgtwo, tc.x ), mix( foretwo, foreone, tc.x ), multip ), 1.0 );
}

void main()
{
  float scale = 10.0;
  float speed = 0.25;
  vec2 direction = normalize( vec2( 1.4, 1.2 ) );

  vec2 tc = vs_out.texcoord;
  vec2 skew = ( processing.resolution / min( processing.resolution.x, processing.resolution.y ) );
  vec2 xuv = ( ( tc ) * skew ) + ( world.time * speed * direction );
  vec2 uv = rotate( vec3( xuv.x, 0.0, xuv.y ), 0.5, normalize( vec3( 0.0, 1.0, 0.0 ) ) ).xz;

  out_color = render( uv, scale, tc );
}