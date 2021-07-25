#version 450 core

uniform sampler2D tex;

in vec2 texcoord;
in vec4 color;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_gbuffer;

void main()
{
  vec4 texsmpl = texture(tex, texcoord);
  float alpha = color.a * texsmpl.r;
  out_color = vec4(color.rgb, alpha);
}