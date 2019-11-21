#version 450 core

uniform sampler2D tex;

in vec2 texcoord;

out vec4 frag_colour;

void main()
{
    frag_colour = texture(tex, texcoord);
}