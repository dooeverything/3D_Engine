#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_tangent;
layout (location = 3) in vec2 in_texCoord;
layout (location = 4) in mat4 in_matrices;

out VS_OUT
{
	vec3 frag_pos;
	vec3 frag_norm;
	vec4 frag_frame_proj;
} vs_out;

out float x;
out float y;
out float z;
out mat4 proj_inverse;

uniform mat4 projection;
uniform mat4 view;

void main()
{	
	x = in_matrices[3][0];
	z = in_matrices[3][1];
	y = in_matrices[3][2];
	proj_inverse = inverse(projection);
	vs_out.frag_pos = vec3(in_matrices * vec4(in_pos, 1.0));
    vs_out.frag_norm = mat3(transpose(inverse(in_matrices))) * in_normal;
	vs_out.frag_frame_proj = projection *  view * in_matrices * vec4(in_pos, 1.0);
	gl_Position = projection * view * in_matrices * vec4(in_pos, 1.0);
    gl_PointSize = 100.0;
}