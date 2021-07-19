#version 450 core

uniform sampler2D tex;

in vec3 normal;
in vec2 texcoord;

out vec4 frag_colour;

void main()
{
  frag_colour = (texture(tex, texcoord) + vec4(normal,1)) / 2.0;
}