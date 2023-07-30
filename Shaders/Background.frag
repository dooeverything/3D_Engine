#version 410 core
out vec4 frag_color;
in vec3 frag_pos;

uniform samplerCube map;

void main()
{
    vec3 color = texture(map, frag_pos).rgb;
    // color = color / (color + vec3(1.0)); // HDR
    // color = pow(color, vec3(1.0/2.2)); // Gamma correction
    frag_color = vec4(color, 1.0);
}