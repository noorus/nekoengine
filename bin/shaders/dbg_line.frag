#version 450 core

in VertexData {
  vec4 color;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

void main()
{
  out_color = vs_out.color;
}