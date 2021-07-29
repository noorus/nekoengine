#version 450 core
#extension GL_ARB_shading_language_include : require

in vec3 normal;
in vec2 texcoord;

layout ( location = 0 ) out vec4 out_color;
layout ( location = 1 ) out vec4 out_gbuffer;

uniform float gamma;
uniform sampler2D tex;

void main()
{
  vec3 color = texture( tex, texcoord ).rgb;
  out_color = vec4( color, 1.0 );
  out_gbuffer = vec4( color, 1.0 );
}