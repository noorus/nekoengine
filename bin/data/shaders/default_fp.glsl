#version 330 core

in vec2 texcoord;

out vec4 frag_colour;

uniform sampler2D tex;

void main()
{
    vec4 pix = texture(tex, texcoord);
    frag_colour = pix;
    //frag_colour = vec4(texcoord.x, 0.0, texcoord.y, 1.0);
}