#version 450 core

#include "inc.buffers.glsl"

out gl_PerVertex {
  vec4 gl_Position;
};

out VertexData {
  vec3 normal;
  vec3 orignormal;
} vs_out;

layout ( location = 0 ) in vec3 vbo_position;
layout ( location = 1 ) in vec3 vbo_normal;
layout ( location = 2 ) in vec2 vbo_texcoord;
layout ( location = 3 ) in vec4 vbo_color;
layout ( location = 4 ) in vec4 vbo_tangent;
layout ( location = 5 ) in vec3 vbo_bitangent;

uniform mat4 model;

void main()
{
  vec3 N = normalize( vbo_normal );
  mat3 normalMatrix = mat3( transpose( inverse( world.camera.view * model ) ) );
  gl_Position = world.camera.view * model * vec4( vbo_position, 1.0 );

  vs_out.normal = vec3( vec4( normalMatrix * N, 0.0 ) );
  vs_out.orignormal = N;
}