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

uniform mat4 model;

void main()
{
  // discard the transformation component of the camera view matrix
  mat4 view = mat4( mat3( world.camera.view ) );
  mat4 modelViewProjection = world.camera.projection * view * model;

  // discard for W, so NDC depth value becomes 1.0, placing the sky behind everything else
  vec4 position = modelViewProjection * vec4( vbo_position.xyz, 1.0 );
  gl_Position = position.xyww;

  vs_out.color = vbo_color;
}