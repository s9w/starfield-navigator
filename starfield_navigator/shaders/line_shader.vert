#version 450 core

layout (location = 1) in vec3 position;
layout (location = 2) in float progress;

layout (std140) uniform ubo_mvp
{
    vec3 camera_pos;
    mat4 view;
    mat4 projection;
};

out float io_progress;
out float io_distance;

void main()
{
   vec4 pos = projection * view * vec4(position, 1.0);
   gl_Position = pos;

   io_progress = progress;
   io_distance = distance(camera_pos, position);
}