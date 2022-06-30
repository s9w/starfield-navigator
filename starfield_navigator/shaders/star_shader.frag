#version 450 core
out vec4 FragColor;

layout (std140) uniform ubo_mvp
{
    vec3 camera_pos;
    mat4 view;
    mat4 projection;
};

in float io_closeness;

void main()
{
    float intensity = clamp(io_closeness, 0.3, 1);
    FragColor = vec4(1, 1, 1, intensity);
} 

