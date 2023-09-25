#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 3) in vec2 in_texCoord;

out vec4 frag_frame_proj;
out vec2 frag_texCoords; 

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
	frag_texCoords = in_texCoord;
	frag_frame_proj = projection *  view * model * vec4(in_pos, 1.0);
	gl_Position = vec4(in_texCoord * 2.0 - 1.0, 0.0, 1.0);
}