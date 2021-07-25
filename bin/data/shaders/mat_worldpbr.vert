#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.buffers.glsl"

layout(location = 0) in vec3 vbo_position;
layout(location = 1) in vec3 vbo_normal;
layout(location = 2) in vec2 vbo_texcoord;

out gl_PerVertex {
  vec4 gl_Position;
};

out VertexData {
  vec3 normal;
  vec2 texcoord;
  vec3 fragpos;
} vs_out;

uniform mat4 model;

void main()
{
  mat4 modelViewProjection = world.projection * world.view * model;
  gl_Position = modelViewProjection * vec4(vbo_position, 1.0);
  vs_out.normal = mat3(model) * vbo_normal;
  vs_out.texcoord = vbo_texcoord;
  vs_out.fragpos = vec3(model * vec4(vbo_position, 1.0));
}