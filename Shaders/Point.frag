#version 330 core
out vec4 frag_color;

float near = 0.5;
float far = 300.0;

// in VS_OUT
// {
// 	vec3 frag_pos;
// 	vec3 frag_norm;
// 	vec4 frag_frame_proj;
// } fs_in;


in vec3 frag_pos;
in mat4 proj;
in float r;


void main()
{
    vec3 n = vec3(0.0);
    n.xy = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(n.xy, n.xy);
	
    if (r2 > 1.0) 
    {
		discard;
	}

    n.z = sqrt(1.0 - r2);
    vec4 p = vec4(frag_pos + n * r, 1.0);
    vec4 clip_p = proj * p;
    float depth1 = (clip_p.z / clip_p.w) * 0.5f + 0.5f;
    // float d = near * far / (far + depth1 * (near - far));

    // float ndc = gl_FragCoord.z;
	float d = near * far / (far + depth1 * (near - far));
	float depth = d / far;

	frag_color = vec4(vec3(depth), 1.0);
}
