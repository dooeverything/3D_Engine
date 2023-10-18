#version 450 core
layout (location = 0) in vec3 in_pos;
layout (location = 0) in vec3 in_norm;

out vec3 frag_pos;
out vec3 frag_norm;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
	frag_pos = vec3(model * vec4(in_pos, 1.0));
    frag_norm = in_norm;  

	gl_Position = projection * view * model * vec4(in_pos, 1.0);
}