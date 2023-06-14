#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 0) in vec3 in_norm;

out vec3 frag_pos;
out vec3 frag_norm;

uniform float outline;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
	frag_pos = vec3(model * vec4(in_pos, 1.0));
	frag_norm = mat3(transpose(inverse(model))) * in_norm;

	gl_Position = projection * view * model * vec4(in_pos + in_norm * outline, 1.0);
}