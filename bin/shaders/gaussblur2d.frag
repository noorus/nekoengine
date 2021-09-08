#version 450 core
#extension GL_ARB_shading_language_include : require

uniform sampler2D tex_image;
uniform bool horizontal;
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

in vec2 in_texcoord;

out vec4 out_color;

void main()
{
  vec2 tex_offset = 1.0 / textureSize( tex_image, 0 );
  vec3 result = texture( tex_image, in_texcoord ).rgb * weight[0];
  if ( horizontal )
  {
    for ( int i = 1; i < 5; ++i )
    {
      result += texture( tex_image, in_texcoord + vec2( tex_offset.x * i, 0.0 ) ).rgb * weight[i];
      result += texture( tex_image, in_texcoord - vec2( tex_offset.x * i, 0.0 ) ).rgb * weight[i];
    }
  }
  else
  {
    for ( int i = 1; i < 5; ++i )
    {
      result += texture( tex_image, in_texcoord + vec2( 0.0, tex_offset.y * i ) ).rgb * weight[i];
      result += texture( tex_image, in_texcoord - vec2( 0.0, tex_offset.y * i ) ).rgb * weight[i];
    }
  }
  out_color = vec4( result, 1.0 );
}
