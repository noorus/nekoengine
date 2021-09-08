#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.buffers.glsl"

uniform sampler2D texMain;
uniform sampler2D texSecondary;

in vec2 texcoord;

out vec4 out_color;

void main()
{
  vec3 smpl = texture( texSecondary, texcoord ).rgb;
  out_color = vec4( smpl, 1.0 );
}
