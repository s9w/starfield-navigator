#version 450 core

layout (location = 1) in vec3 position;

layout (std140) uniform ubo_mvp
{
//   vec4 lookat_pos;
    mat4 view;
    mat4 projection;
    //mat4 m_pv;
    //mat4 m_pv_inverse;
    //vec3 camera_pos;
};

void main()
{
   vec4 pos = projection * view * vec4(position, 1.0);
   pos /= pos.w;
   gl_Position = pos;
}