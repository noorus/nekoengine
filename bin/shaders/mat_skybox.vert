#version 450 core

#include "inc.buffers.glsl"

out gl_PerVertex {
  vec4 gl_Position;
};

layout ( location = 0 ) in vec3 vbo_position;
layout ( location = 1 ) in vec3 vbo_normal;
layout ( location = 2 ) in vec2 vbo_texcoord;
layout ( location = 3 ) in vec4 vbo_color;

uniform mat4 model;

out VertexData {
  vec3 normal;
  vec2 texcoord;
  vec3 fragpos;
  vec4 color;
} vs_out;

void main()
{
  // discard the transformation component of the camera view matrix
  mat4 view = mat4( mat3( world.camera.view ) );
  mat4 modelViewProjection = world.camera.projection * view * model;

  // discard for W, so NDC depth value becomes 1.0, placing the sky behind everything else
  vec4 position = modelViewProjection * vec4( vbo_position.xyz, 1.0 );
  gl_Position = position.xyww;

  vs_out.normal = mat3( model ) * vec3( vbo_normal.x, vbo_normal.y, vbo_normal.z );
  vs_out.texcoord = vbo_texcoord;
  vs_out.fragpos = vec3( model * vec4( vbo_position, 1.0 ) );
  vs_out.color = vbo_color;
}