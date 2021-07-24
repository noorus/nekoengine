#version 450 core

uniform sampler2D texAlbedo;
uniform sampler2D texHeight;
uniform sampler2D texMetallic;
uniform sampler2D texNormal;

in VertexData {
  vec3 normal;
  vec2 texcoord;
} vs_out;

out vec4 out_color;

void main()
{
  out_color = texture(texNormal, vs_out.texcoord);
}