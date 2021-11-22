#version 450 core

#include "inc.buffers.glsl"

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
};

out VertexData {
  vec4 orientation;
  vec2 size;
  vec4 color;
  float id;
} vs_out;

layout ( location = 0 ) in vec3 vbo_position;
layout ( location = 1 ) in vec4 vbo_orientation;
layout ( location = 2 ) in vec2 vbo_size;
layout ( location = 3 ) in vec4 vbo_color;

uniform mat4 model;

void main()
{
  gl_Position = model * vec4( vbo_position, 1.0 );
  vs_out.orientation = vbo_orientation;
  vs_out.size = vbo_size;
  vs_out.color = vbo_color;
  vs_out.id = float( gl_VertexID );
}