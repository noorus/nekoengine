#version 450 core

out gl_PerVertex{
  vec4 gl_Position;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 vboTexcoord;
layout(location = 2) in vec4 vboColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texcoord;
out vec4 color;

void main()
{
  mat4 modelViewProjection = projection * view * model;
  gl_Position = modelViewProjection * vec4(position, 1.0);
  texcoord = vboTexcoord;
  color = vboColor;
}