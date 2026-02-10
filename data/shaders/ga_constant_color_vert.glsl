#version 330 core

uniform mat4 u_mvp;

layout(location = 0) in vec3 in_vertex;

void main()
{
    gl_Position = u_mvp * vec4(in_vertex, 1.0);
}
