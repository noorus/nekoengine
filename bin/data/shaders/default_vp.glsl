#version 400

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    mat4 modelViewProjection = projection * model;
    gl_Position = modelViewProjection * vec4(position, 0.0, 1.0);
}