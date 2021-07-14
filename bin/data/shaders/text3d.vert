#version 450 core

out gl_PerVertex{
  vec4 gl_Position;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 vboTexcoord;
layout(location = 2) in vec4 vboColor;

layout (std140, binding = 0) buffer World
{
  mat4 projection;
  mat4 view;
} world;

uniform mat4 model;

out vec2 texcoord;
out vec4 color;

void main()
{
  mat4 modelViewProjection = world.projection * world.view * model;
  gl_Position = modelViewProjection * vec4(position, 1.0);
  texcoord = vboTexcoord;
  color = vboColor;
}