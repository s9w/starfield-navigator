#version 450 core
out vec4 FragColor;

uniform float time;

{{ubo_code}}

void main()
{
    FragColor = vec4( vec3(1), 0.5);
} 

