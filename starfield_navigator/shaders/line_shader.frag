#version 450 core
out vec4 FragColor;

uniform float time;

{{ubo_code}}

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
        float intensity = ease(io_distance, 20, 220, 1, 0.2);
        vec3 base_color = vec3(0.5, 0.5, 1.0);
        FragColor = vec4( intensity * base_color, 0.5);
    }
    else
    {
        float alpha = 1-step(0.5, mod(io_progress - time, 1));
        FragColor = vec4(1, 0, 0, alpha);
    }
} 

