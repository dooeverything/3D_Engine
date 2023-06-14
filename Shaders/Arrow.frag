#version 330 core
out vec4 frag_color;

in vec3 frag_pos;
in vec3 frag_norm;

uniform vec3 object_color;

void main()
{
	frag_color = vec4(object_color, 1.0);
}