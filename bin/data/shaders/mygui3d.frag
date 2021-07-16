#version 450 core

uniform sampler2D tex;

in vec4 color;
in vec2 texcoord;

out vec4 frag_colour;

void main()
{
  vec4 smpl = texture(tex, texcoord);
  frag_colour = vec4(smpl.b, smpl.g, smpl.r, smpl.a) * color;
}