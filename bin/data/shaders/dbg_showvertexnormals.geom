#version 450 core

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
} gs_in[];

layout (std140, binding = 0) buffer World
{
  mat4 projection;
  mat4 view;
} world;

uniform mat4 model;

const float c_linemagnitude = 1.2;

void lineFor(int index)
{
  gl_Position = world.projection * gl_in[index].gl_Position;
  EmitVertex();
  gl_Position = world.projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * c_linemagnitude);
  EmitVertex();
  EndPrimitive();
}

void main()
{
  lineFor(0);
  lineFor(1);
  lineFor(2);
}