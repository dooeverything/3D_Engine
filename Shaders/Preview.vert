#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_tangent;
layout (location = 3) in vec2 in_texCoord;

out VS_OUT
{
	vec3 frag_pos;
	vec3 frag_norm;
	vec2 frag_texCoord;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vs_out.frag_pos = vec3(model * vec4(in_pos, 1.0));
    vs_out.frag_norm = mat3(transpose(inverse(model))) * in_normal;
    vs_out.frag_texCoord = in_texCoord;
    gl_Position = projection * view * model  * vec4(in_pos, 1.0);
}