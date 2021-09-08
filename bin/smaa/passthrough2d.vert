#version 450 core

out gl_PerVertex{
  vec4 gl_Position;
};

layout ( location = 0 ) in vec2 vbo_position;
layout ( location = 1 ) in vec2 vbo_texcoord;

out vec2 out_texcoord;

void main()
{
  gl_Position = vec4( vbo_position.x, vbo_position.y, 0.0, 1.0 );
  out_texcoord = vbo_texcoord;
}