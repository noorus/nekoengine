#version 450 core

#include "inc.buffers.glsl"

layout ( location = 0 ) in vec3 vbo_position;
layout ( location = 1 ) in vec3 vbo_normal;
layout ( location = 2 ) in vec2 vbo_texcoord;
layout ( location = 3 ) in vec4 vbo_color;
layout ( location = 4 ) in vec4 vbo_tangent;
layout ( location = 5 ) in vec3 vbo_bitangent;

out gl_PerVertex {
  vec4 gl_Position;
};

out VertexData {
  vec3 normal;
  vec3 fragpos;
} vs_out;

uniform mat4 model;

void main()
{
  mat4 modelViewProjection = world.camera.projection * world.camera.view * model;

  gl_Position = modelViewProjection * vec4( vbo_position, 1.0 );

  vs_out.normal = mat3( model ) * vbo_normal;
  vs_out.fragpos = vec3( model * vec4( vbo_position, 1.0 ) );
}