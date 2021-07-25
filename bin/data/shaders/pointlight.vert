#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.buffers.glsl"

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
};

out VertexData {
  vec4 lightColor;
} vs_out;

layout ( location = 0 ) in vec3 vbo_position;
layout ( location = 1 ) in vec4 vbo_color;

uniform mat4 model;

void main()
{
  mat4 modelView = world.view * model;
  gl_Position = modelView * vec4( vbo_position, 1.0 );
  // gl_PointSize = 35.0;
  vs_out.lightColor = vbo_color;
}