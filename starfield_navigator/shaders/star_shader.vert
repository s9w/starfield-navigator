#version 450 core

layout (location = 1) in vec3 position;

layout (std140) uniform ubo_mvp
{
    vec3 camera_pos;
    mat4 view;
    mat4 projection;
};

out float io_distance;

void main()
{
   vec4 pos = projection * view * vec4(position, 1.0);
   pos /= pos.w;
   gl_Position = pos;
   
   float distance_from_cam = distance(position, camera_pos);
   gl_PointSize = 500/distance_from_cam;
   io_distance = distance_from_cam;
}