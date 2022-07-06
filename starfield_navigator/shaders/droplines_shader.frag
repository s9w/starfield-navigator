#version 450 core
out vec4 FragColor;

in float io_alpha;

void main()
{
    FragColor = vec4( vec3(1), io_alpha);
} 

