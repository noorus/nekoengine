#version 450 core

out gl_PerVertex{
  vec4 gl_Position;
};

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 vboTexcoord;

out vec2 texcoord;

void main()
{
  gl_Position = vec4(position.x, position.y, 0.0, 1.0);
  texcoord = vboTexcoord;
}