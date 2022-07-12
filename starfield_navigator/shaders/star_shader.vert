#version 450 core

layout (location = 1) in vec3 position;

{{ubo_code}}

out float io_distance;
out vec3 io_color;

void main()
{
   vec4 pos = projection * view * vec4(0.3*position + ssbo_data[gl_InstanceID].position, 1.0);
   gl_Position = pos;
   io_color = ssbo_data[gl_InstanceID].color;
}