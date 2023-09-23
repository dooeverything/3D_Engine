#version 330 core
// out vec4 frag_color;

float near = 0.1;
float far = 100.0;

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

    vec4 eye = vec4(frag_pos + n * r, 1.0); // EYE
    vec4 clip = proj * eye; // CLIP 
    
    float z = (clip.z / clip.w)*0.5 + 0.5; // NDC converted from [-1,1] to [0,1]
	gl_FragDepth  = z; 
}
