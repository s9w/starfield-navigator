#version 450 core
out vec4 FragColor;

uniform float time;

layout (std140) uniform ubo_mvp
{
    vec3 camera_pos;
    mat4 view;
    mat4 projection;
};

in float io_progress;


void main()
{
    float alpha = 1-step(0.5, mod(io_progress - time, 1));
    FragColor = vec4(1, 0, 0, alpha);
} 

