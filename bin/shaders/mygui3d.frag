#version 450 core

uniform sampler2D tex;

in vec4 color;
in vec2 texcoord;

out vec4 out_color;

void main()
{
  vec4 smpl = texture(tex, texcoord);
  out_color = vec4(smpl.b, smpl.g, smpl.r, smpl.a) * color;
}