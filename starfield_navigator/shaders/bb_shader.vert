#version 450 core

layout (location = 1) in vec3 position;
out float io_progress;
{{ubo_code}}

void main()
{
   // bb_trafos[gl_InstanceID] *
   vec4 pos = projection * view * bb_trafos[gl_InstanceID] * vec4(position, 1.0);
   gl_Position = pos;
   io_progress = length(position);
}