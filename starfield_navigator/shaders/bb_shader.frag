#version 450 core
out vec4 FragColor;

in float io_progress;
uniform float time;

void main()
{
    float brightness = step(0.5, mod(30*io_progress + 0.2*time, 1.0));
    FragColor = vec4(vec3(brightness), 1.0);
} 

