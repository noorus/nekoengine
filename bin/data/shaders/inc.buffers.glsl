#ifndef WORLD_INCLUDE_GLSL
#define WORLD_INCLUDE_GLSL

#ifdef __cplusplus

#include "neko_types.h"

namespace neko::uniforms {

#define NEKO_DECLARE_UNIFORMBLOCK(i,n) __declspec(align(16)) struct n
#define NEKO_UNIFORM_INSTANCE(n)
#pragma pack( push, 1 )

#else

#define NEKO_DECLARE_UNIFORMBLOCK(i,n) layout( std430, binding = i ) buffer n
#define NEKO_UNIFORM_INSTANCE(n) n

#endif

  const int c_pointLightCount = 16;

  struct PointLight {
    vec4 position;
    vec4 color;
    vec4 dummy;
  };

  struct Camera {
    vec4 position;
    mat4 projection;
    mat4 view;
  };

  NEKO_DECLARE_UNIFORMBLOCK( 0, World )
  {
    float time;
    float d0;
    float d1;
    float d2;
    Camera camera;
    PointLight pointLights[c_pointLightCount];
  } NEKO_UNIFORM_INSTANCE( world );

  const int c_gaussianBlurSamples = 21;

  NEKO_DECLARE_UNIFORMBLOCK( 1, Processing )
  {
    float gaussianKernel[c_gaussianBlurSamples];
    float gaussianZ;
    float d0;
    float d1;
    vec4 ambient;
  } NEKO_UNIFORM_INSTANCE( processing );

#ifdef __cplusplus

#pragma pack( pop )

}

#endif

#endif