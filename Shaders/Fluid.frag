#version 330 core
out vec4 frag_color;
// out float frag_depth;

in VS_OUT
{
	vec3 frag_pos;
	vec3 frag_norm;
	vec4 frag_frame_proj;
} fs_in;

uniform vec3 view_pos;
uniform vec3 light_pos;
// uniform samplerCube skybox;
uniform sampler2D map;

vec3 refraction(vec3 N, vec3 V, float r)
{
	float cos_i = dot(N,V);
	float k = 1 - r*r*(1-cos_i*cos_i);
	
	if(k < 0.0)
		return vec3(0.0);
	else
	{
		float a = cos_i*r - sqrt(k);
		return normalize(-V*r + N*a);
	}
}

void main()
{
	vec3 proj_coords = fs_in.frag_frame_proj.xyz / fs_in.frag_frame_proj.w;
    proj_coords = proj_coords * 0.5 + 0.5; 

	vec3 normal = texture2D(map, proj_coords.xy).rgb;

	frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}
