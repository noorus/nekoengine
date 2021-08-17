#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.buffers.glsl"

layout ( triangles ) in;
layout ( line_strip, max_vertices = 6 ) out;

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
  vec3 tangent;
} gs_in[];

out VertexData
{
  vec3 tangent;
} gs_out;

uniform mat4 model;

const float c_linemagnitude = 1.2;

void lineForVertex( mat4 projection, int index )
{
  gl_Position = projection * gl_in[index].gl_Position;
  EmitVertex();

  gl_Position = projection * ( gl_in[index].gl_Position + vec4( gs_in[index].tangent, 0.0 ) * c_linemagnitude );
  EmitVertex();

  EndPrimitive();
}

void main()
{
  lineForVertex( world.camera.projection, 0 );
  lineForVertex( world.camera.projection, 1 );
  lineForVertex( world.camera.projection, 2 );
}