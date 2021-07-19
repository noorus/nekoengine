#version 450 core

out gl_PerVertex {
  vec4 gl_Position;
};

out VertexData {
  vec3 normal;
} vs_out;

layout(location = 0) in vec3 vbo_position;
layout(location = 1) in vec3 vbo_normal;
layout(location = 2) in vec2 vbo_texcoord;

layout (std140, binding = 0) buffer World
{
  mat4 projection;
  mat4 view;
} world;

uniform mat4 model;

void main()
{
  mat3 normalmat = mat3(transpose(inverse(world.view * model)));
  vs_out.normal = vec3(vec4(normalmat * vbo_normal, 0.0));
  gl_Position = world.view * model * vec4(vbo_position, 1.0);
}