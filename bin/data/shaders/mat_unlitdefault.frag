#version 450 core

in vec3 normal;
in vec2 texcoord;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_gbuffer;

uniform sampler2D tex;

void main()
{
  vec3 color = texture(tex, texcoord).rgb;

  float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
  if ( brightness > 1.0 )
    out_gbuffer = vec4(color, 1.0);
  else
    out_gbuffer = vec4(0.0, 0.0, 0.0, 1.0);

  out_color = vec4(color, 1.0);
}