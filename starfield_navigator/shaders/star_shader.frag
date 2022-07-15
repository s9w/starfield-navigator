#version 450 core
out vec4 FragColor;

in vec3 io_color;
in float io_blink_weight;
uniform float time;

{{ubo_code}}

float weight(float value, float weight)
{
    return mix(1, value, weight);
}

void main()
{
    float blink_factor = step(0.5, mod(2*time, 1.0));
    float blink_effective = mix(1, blink_factor, io_blink_weight);

    float alpha = weight(blink_effective * 1.0, 0.75);
    FragColor = vec4(io_color, alpha);
} 

