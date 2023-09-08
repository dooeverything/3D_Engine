#version 330 core
out vec4 frag_color;

float near = 0.1;
float far = 100.0;

in VS_OUT
{
	vec3 frag_pos;
	vec3 frag_norm;
	vec4 frag_frame_proj;
} fs_in;

void main()
{
	float ndc = gl_FragCoord.z;
	float d = near * far / (far + ndc * (near - far));
	float depth = d / far;
	frag_color = vec4(vec3(depth), 1.0);
}
