#version 450 core

layout (location = 1) in vec3 position;
layout (location = 2) in float progress;

{{ubo_code}}

out float io_progress;
out float io_distance;

void main()
{
   vec4 pos = projection * view * vec4(position, 1.0);
   gl_Position = pos;

   io_progress = progress;
   io_distance = distance(camera_pos, position);
}