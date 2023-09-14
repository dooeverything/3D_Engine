#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 3) in vec2 in_texCoord;

out vec2 texCoords; 

void main()
{
	texCoords = in_texCoord;
	gl_Position = vec4(in_pos.x, in_pos.y, 0.0, 1.0);
}