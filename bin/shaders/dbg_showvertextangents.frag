#version 450 core

in vec3 tangent;

layout ( location = 0 ) out vec4 out_color;

void main()
{
  //vec3 color = abs( normal.rgb );
  //out_color = vec4( color, 1.0 );
  out_color = vec4( 0.9, 0.7, 0.0, 1.0 );
}