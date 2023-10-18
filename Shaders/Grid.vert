// Reference: 3D Graphics Rendering Cookbook, Chapter5 
// https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook/tree/master/Chapter5/GL01_Grid

#version 450 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;

out vec3 frag_pos;
out vec3 frag_norm;
out vec2 out_cam;
out vec2 out_uv;

uniform vec3 cam_pos;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	float grid_size = 100.0;

	frag_pos = vec3(model * vec4(in_pos, 1.0));
    frag_norm = in_norm;  

	vec3 pos = in_pos * grid_size;
	pos.x += cam_pos.x;
	pos.z += cam_pos.z;
	out_cam = cam_pos.xz;

	gl_Position = projection * view * model * vec4(pos, 1.0);
	out_uv = pos.xz;
}