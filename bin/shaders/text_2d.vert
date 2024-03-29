#version 450 core

#include "inc.buffers.glsl"

out gl_PerVertex {
  vec4 gl_Position;
};

layout ( location = 0 ) in vec3 vbo_position;
layout ( location = 1 ) in vec2 vbo_texcoord;
layout ( location = 2 ) in vec4 vbo_color;

uniform mat4 model;

out VertexData {
  vec2 texcoord;
  vec3 fragpos;
  vec4 color;
} vs_out;

void main()
{
  mat4 modelProjection = processing.textproj * model;

  gl_Position = modelProjection * vec4( vbo_position, 1.0 );

  vs_out.texcoord = vbo_texcoord;
  vs_out.fragpos = vec3( model * vec4( vbo_position, 1.0 ) );
  vs_out.color = vbo_color;
}