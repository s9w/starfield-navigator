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
in float io_distance;

float ease(float x, float x0, float x1, float y0, float y1){
  float dx = x1-x0;
  float dy = y1-y0;
  float xeff = clamp((x-x0)/dx, 0.0, 1.0);
  return y0 + xeff * dy;
}

void main()
{
    if(time<0)
    {
        float intensity = ease(io_distance, 20, 150, 1, 0.5);
        FragColor = vec4(0.5, 0.5, 1.0, 0.5 * intensity);
    }
    else
    {
        float alpha = 1-step(0.5, mod(io_progress - time, 1));
        FragColor = vec4(1, 0, 0, alpha);
    }
} 

