#version 400 core

uniform mat4 u_mvp;

layout(location = 0) in vec3 in_vertex;
layout(location = 2) in vec2 in_texcoord0;

out vec2 texcoord0;

void main()
{
    gl_Position = u_mvp * vec4(in_vertex, 1.0);
    texcoord0 = in_texcoord0;
}
