#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.world.glsl"

out gl_PerVertex {
  vec4 gl_Position;
};

out VertexData {
  vec3 normal;
  vec3 orignormal;
} vs_out;

layout(location = 0) in vec3 vbo_position;
layout(location = 1) in vec3 vbo_normal;
layout(location = 2) in vec2 vbo_texcoord;

uniform mat4 model;

void main()
{
  mat3 normalMatrix = mat3(transpose(inverse(world.view * model)));
  vs_out.normal = vec3(vec4(normalMatrix * vbo_normal, 0.0));
  vs_out.orignormal = vbo_normal;
  gl_Position = world.view * model * vec4(vbo_position, 1.0);
}