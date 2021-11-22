#version 450 core

out gl_PerVertex {
  vec4 gl_Position;
};

layout ( location = 0 ) in vec3 vbo_position;
layout ( location = 1 ) in vec4 vbo_color;
layout ( location = 2 ) in vec2 vbo_texcoord;

uniform float yscale;

out VertexData {
  vec4 color;
  vec2 texcoord;
} vs_out;

void main()
{
  gl_Position = vec4( vbo_position.x, vbo_position.y * yscale, vbo_position.z, 1.0 );
  vs_out.texcoord = vbo_texcoord;
  vs_out.color = vbo_color;
}