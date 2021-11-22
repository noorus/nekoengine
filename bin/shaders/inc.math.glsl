#ifndef MATH_INCLUDE_GLSL
#define MATH_INCLUDE_GLSL

const float M_PI = 3.14159265358979323846264338327950288;
const float M_PI_2 = 1.57079632679489661923132169163975144;
const float M_PI_4 = 0.785398163397448309615660845819875721;

vec3 quat_multiply( vec4 q, vec3 v )
{
  return ( v + 2.0 * cross( q.xyz, cross( q.xyz, v ) + q.w * v ) );
}

vec4 quat_fromAngleAxis( float angle, vec3 axis )
{
  float st = sin( angle * 0.5 );
  return vec4( axis.x * st, axis.y * st, axis.z * st, cos( angle * 0.5 ) );
}

#endif