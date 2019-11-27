#version 450 core

uniform sampler2D tex;

in vec2 texcoord;
in vec4 color;

out vec4 frag_colour;

void main()
{
  vec4 texsmpl = texture(tex, texcoord);
  float alpha = color.a * texsmpl.r;
  frag_colour = vec4(color.rgb, alpha);
}