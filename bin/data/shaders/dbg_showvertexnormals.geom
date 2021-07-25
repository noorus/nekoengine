#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.buffers.glsl"

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in gl_PerVertex
{
  vec4 gl_Position;
} gl_in[];

out gl_PerVertex
{
  vec4 gl_Position;
};

in VertexData
{
  vec3 normal;
  vec3 orignormal;
} gs_in[];

out VertexData
{
  vec3 normal;
} gs_out;

uniform mat4 model;

const float c_linemagnitude = 1.2;

void lineFor(int index)
{
  gl_Position = world.projection * gl_in[index].gl_Position;
  gs_out.normal = gs_in[index].orignormal;
  EmitVertex();
  gl_Position = world.projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * c_linemagnitude);
  gs_out.normal = gs_in[index].orignormal;
  EmitVertex();
  EndPrimitive();
}

void main()
{
  lineFor(0);
  lineFor(1);
  lineFor(2);
}