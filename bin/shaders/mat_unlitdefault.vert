#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.buffers.glsl"

out gl_PerVertex{
  vec4 gl_Position;
};

layout ( location = 0 ) in vec3 vbo_position;
layout ( location = 1 ) in vec3 vbo_normal;
layout ( location = 2 ) in vec2 vbo_texcoord;

uniform mat4 model;

out vec3 out_normal;
out vec2 out_texcoord;

void main()
{
  mat4 modelViewProjection = world.camera.projection * world.camera.view * model;

  gl_Position = modelViewProjection * vec4( vbo_position, 1.0 );

  out_normal = vbo_normal;
  out_texcoord = vbo_texcoord;
}