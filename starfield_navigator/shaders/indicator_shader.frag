#version 450 core
out vec4 FragColor;

uniform float time;

layout (std140) uniform ubo_mvp
{
    vec3 camera_pos;
    mat4 view;
    mat4 projection;
};

void main()
{
    FragColor = vec4( vec3(1), 0.5);
} 

