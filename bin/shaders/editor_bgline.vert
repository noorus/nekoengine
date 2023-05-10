#version 450 core

#include "inc.buffers.glsl"

out gl_PerVertex {
  vec4 gl_Position;
};

out VertexData {
  vec4 color;
} vs_out;

layout ( location = 0 ) in vec3 vbo_position;
layout ( location = 1 ) in vec4 vbo_color;

void main()
{
  mat4 modelViewProjection = world.camera.projection * world.camera.view;

  gl_Position = modelViewProjection * vec4( vbo_position.xyz, 1.0 );

  vs_out.color = vbo_color;
}