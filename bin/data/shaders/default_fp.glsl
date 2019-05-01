#version 450

in vec2 texcoord;

out vec4 frag_colour;

void main()
{
    frag_colour = vec4(texcoord.x, 0.0, texcoord.y, 1.0);
}