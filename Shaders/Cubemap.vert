#version 450 core
layout (location=0) in vec3 in_pos;

out vec3 frag_pos;

uniform mat4 PV;

void main()
{
    frag_pos = in_pos;
    gl_Position = PV * vec4(in_pos, 1.0);
}