#version 450 core
layout (location = 0) in vec3 in_pos;
layout (location = 3) in vec2 in_texCoord;

out vec4 tcs_pos_light;
out vec3 tcs_pos_model;
out vec2 tcs_texCoords; 

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 light_matrix;

void main()
{
	tcs_texCoords = in_texCoord;
	tcs_pos_light = light_matrix * model * vec4(in_pos, 1.0);
	tcs_pos_model = vec3(model * vec4(in_pos, 1.0));
	gl_Position = projection * view * model  * vec4(in_pos, 1.0);
}