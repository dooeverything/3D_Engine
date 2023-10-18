#version 450 core
layout (location = 0) in vec3 in_pos;
layout (location = 4) in mat4 in_matrices;

out vec3 frag_pos;
out mat4 proj;
out float r;

uniform mat4 projection;
uniform mat4 view;

uniform float point_radius;
uniform float point_scale;

void main()
{	
	proj = projection;
    r = point_radius;
    frag_pos = vec3(in_matrices * vec4(in_pos, 1.0));
	gl_Position = projection * view * in_matrices * vec4(in_pos, 1.0);
    gl_PointSize = point_scale * (point_radius / gl_Position.w);
}