#version 450 core

layout (location = 1) in vec3 position;

layout (std140) uniform ubo_mvp
{
    vec3 camera_pos;
    mat4 view;
    mat4 projection;
};

out float io_closeness;

void main()
{
   vec4 pos = projection * view * vec4(position, 1.0);
   pos /= pos.w;
   gl_Position = pos;
   
   float distance_from_cam = distance(position, camera_pos);
   float closeness = 1.0 - distance_from_cam / 130.0;
   gl_PointSize = clamp(closeness, 0.4, 1.0) * 8;
   io_closeness = closeness;
}