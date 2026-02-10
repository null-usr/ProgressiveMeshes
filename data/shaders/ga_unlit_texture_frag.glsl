#version 400 core

uniform sampler2D u_texture;

in vec2 texcoord0;
out vec4 FragColor;

void main()
{
    FragColor = texture(u_texture, texcoord0);
}
