#version 450 core

uniform sampler2D tex;

in vec2 texcoord;

out vec4 frag_colour;

void main()
{
    vec4 asdasd = vec4(0.9,0.11,0.42,1.0);
    vec4 clr = texture(tex, texcoord);
    frag_colour = vec4(asdasd.rgb, clr.r);
}