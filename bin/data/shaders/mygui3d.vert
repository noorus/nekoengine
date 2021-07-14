#version 450 core

out gl_PerVertex{
  vec4 gl_Position;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 vboColor;
layout(location = 2) in vec2 vboTexcoord;

layout (std140, binding = 0) buffer World
{
  mat4 projection;
  mat4 view;
} world;

uniform float yscale;

out vec4 color;
out vec2 texcoord;

void main()
{
  texcoord = vboTexcoord;
  color = vboColor;
  vec4 vpos = vec4(position, 1.0);
  vpos.y *= yscale;
  gl_Position = vpos;
}