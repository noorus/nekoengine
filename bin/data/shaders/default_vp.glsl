#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 vboTexcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texcoord;

void main()
{
    mat4 modelViewProjection = projection * model;
    gl_Position = modelViewProjection * vec4(position, 0.0, 1.0);
    texcoord = vboTexcoord;
}