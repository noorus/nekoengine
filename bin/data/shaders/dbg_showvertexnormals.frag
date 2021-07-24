#version 450 core

in vec3 normal;

out vec4 out_color;

void main()
{
  // vec3 color = ( normal.rgb + vec3( 1.0 ) ) * vec3( 0.5 );
  vec3 color = abs( normal.rgb );
  out_color = vec4( color, 1.0 );
}