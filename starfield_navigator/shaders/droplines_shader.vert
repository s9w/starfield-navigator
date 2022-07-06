#version 450 core

layout (location = 1) in vec3 position;

uniform float range;

out float io_alpha;

{{ubo_code}}

void main()
{
   float dist_to_selected = distance(position.xy, selected_system_pos.xy);

   vec4 screen_pos = projection * view * vec4(position, 1.0);
   gl_Position = screen_pos;

   io_alpha = 0.2 * smoothstep(range+5, range-5, dist_to_selected);
}
