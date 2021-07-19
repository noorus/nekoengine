#version 450 core

uniform sampler2D tex;

in vec3 normal;
in vec2 texcoord;

out vec4 out_color;

void main()
{
  out_color = texture(tex, texcoord);
}