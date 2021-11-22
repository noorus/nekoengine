#version 450 core

in VertexData {
  vec4 color;
  vec2 texcoord;
} vs_out;

uniform sampler2D tex;

in vec4 color;
in vec2 texcoord;

layout ( location = 0 ) out vec4 out_color;

void main()
{
  // mygui's textures in-memory have reversed channel order
  vec4 diffuse = texture( tex, vs_out.texcoord ).bgra;
  out_color = diffuse * vs_out.color;
}