#version 450 core
out vec4 FragColor;

in vec3 io_color;

void main()
{
    FragColor = vec4(io_color, 1);
} 

