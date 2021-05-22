#version 450 core

out gl_PerVertex{
  vec4 gl_Position;
};

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 vboTexcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texcoord;

void main()
{
  mat4 modelProjection = projection * model;
  gl_Position = modelProjection * vec4(position, 0.0, 1.0);
  texcoord = vboTexcoord;
}