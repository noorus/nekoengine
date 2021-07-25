#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.buffers.glsl"

out gl_PerVertex{
  vec4 gl_Position;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vboNormal;
layout(location = 2) in vec2 vboTexcoord;

uniform mat4 model;

out vec3 normal;
out vec2 texcoord;

void main()
{
  mat4 modelViewProjection = world.projection * world.view * model;
  gl_Position = modelViewProjection * vec4(position, 1.0);
  normal = vboNormal;
  texcoord = vboTexcoord;
}