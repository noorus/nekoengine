#version 450 core

#include "inc.buffers.glsl"

out gl_PerVertex {
  vec4 gl_Position;
};

layout ( location = 0 ) in vec2 vbo_position;
layout ( location = 1 ) in vec2 vbo_texcoord;

uniform mat4 model;

out VertexData {
  vec2 texcoord;
} vs_out;

void main()
{
  gl_Position = vec4( vbo_position.x, vbo_position.y, 0.0, 1.0 );
  vs_out.texcoord = vbo_texcoord;
}