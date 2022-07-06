#version 450 core

layout (location = 1) in vec3 position;

{{ubo_code}}

void main()
{
   vec4 pos = projection * view * vec4(position, 1.0);
   gl_Position = pos;
}