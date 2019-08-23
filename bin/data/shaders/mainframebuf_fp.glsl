#version 330 core

in vec2 texcoord;

out vec4 frag_colour;

uniform sampler2D screenTexture;

void main()
{
    frag_colour = texture(screenTexture, texcoord);
}