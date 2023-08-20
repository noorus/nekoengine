#version 450 core

#include "inc.buffers.glsl"

in VertexData {
  vec2 texcoord;
} vs_out;

layout ( location = 0 ) out vec4 out_color;

uniform sampler2DArray tex;
uniform int tex_layer;

void main()
{
  const float w = 32.0;
  const float h = 32.0;

  vec2 alpha = 0.7 * vec2(dFdx(vs_out.texcoord.x), dFdy(vs_out.texcoord.y));
  vec2 x = fract(vs_out.texcoord);
  vec2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) +
            clamp(0.5 / alpha * (x - 1.0) + 0.5,
                  0.0, 0.5);
 
  vec2 texCoord = (floor(vs_out.texcoord) + x_) / vec2(w, h);

  vec4 diffuse = texture( tex, vec3( texCoord, tex_layer ), gl_SampleID ).rgba;
  out_color = diffuse;
}
