#version 450 core
out vec4 FragColor;

layout (std140) uniform ubo_mvp
{
    vec3 camera_pos;
    mat4 view;
    mat4 projection;
};

in float io_distance;

void main()
{
    vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
    float dist = dot(circCoord, circCoord);

    if (dot(circCoord, circCoord) > 1.0) {
        discard;
    }


    // if (dot(circCoord, circCoord) > 1.2) {
    //     discard;
    // }
    // float alpha = smoothstep(1.2, 0.8, dist);

    float intensity = clamp(1-io_distance/150, 0.5, 1);
    FragColor = vec4(vec3(intensity), 1);
} 

