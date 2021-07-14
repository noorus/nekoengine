#version 450 core

uniform sampler2D tex;

in vec4 color;
in vec2 texcoord;

out vec4 frag_colour;

void main()
{
  frag_colour = texture2D(tex, texcoord) * color;
}